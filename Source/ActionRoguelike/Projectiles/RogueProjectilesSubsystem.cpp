// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectilesSubsystem.h"

#include "ActionRoguelike.h"
#include "GenericTeamAgentInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannelAccessor.h"
#include "NiagaraDataChannelAccessContext.h"
#include "NiagaraDataChannelFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Core/RogueGameState.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(LightweightProjectilesCount, TEXT("Game/DataOnlyProjectileCount"));

bool URogueProjectilesSubsystem::HasSynchronizedTime() const
{
	const ARogueGameState* GS = GetWorld()->GetGameState<ARogueGameState>();
	return GS && GS->HasSynchronizedServerTime();
}


uint32 URogueProjectilesSubsystem::AllocatePredictionSession()
{
	check(HasAuthority());
	check(NextPredictionSession != 0);
	return NextPredictionSession++;
}


int64 URogueProjectilesSubsystem::CreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor)
{
	if (!HasAuthority() || !IsValid(ProjectileConfig) || InPosition.ContainsNaN() || InDirection.ContainsNaN() || InDirection.IsNearlyZero())
	{
		return 0;
	}
	return LaunchProjectile(InPosition, InDirection, ProjectileConfig, InstigatorActor, {}, nullptr).ID;
}


FProjectileLaunch URogueProjectilesSubsystem::LaunchProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig,
	AActor* InstigatorActor, FProjectilePredictionKey Key, URogueActionComponent* RequestingComponent)
{
	check(HasAuthority());
	check(ProjectileConfig);
	check(NextProjectileID > 0 && NextProjectileID < MAX_int64);
	check(FMath::IsFinite(ProjectileConfig->InitialSpeed) && ProjectileConfig->InitialSpeed > 0);
	check(FMath::IsFinite(ProjectileConfig->Lifespan) && ProjectileConfig->Lifespan > 0);
	ARogueGameState* GS = GetWorld()->GetGameState<ARogueGameState>();
	check(GS);

	FProjectileLaunch Launch;
	Launch.ID = NextProjectileID++;
	Launch.PredictionKey = Key;
	Launch.Position = InPosition;
	Launch.Velocity = InDirection.GetSafeNormal() * ProjectileConfig->InitialSpeed;
	Launch.SimulationTime = GetServerTime();
	Launch.ExpirationTime = Launch.SimulationTime + FMath::Max(0.01f, ProjectileConfig->Lifespan);
	Launch.ConfigDataAsset = ProjectileConfig;
	Launch.InstigatorActor = InstigatorActor;

	FProjectileItem& Item = GS->ProjectileData.Items.AddDefaulted_GetRef();
	Item.Projectile = Launch;
	GS->ProjectileData.MarkItemDirty(Item);
	GS->ForceNetUpdate();

	FProjectileInstance& Instance = ProjectileInstances.AddDefaulted_GetRef();
	Instance.Launch = Launch;
	Instance.Position = InPosition;
	Instance.SimulationTime = Launch.SimulationTime;
	Instance.RequestingComponent = RequestingComponent;
	StartTracer(Instance);

	FProjectileUpdate Accepted;
	Accepted.Projectile = Launch;
	QueueOwnerUpdate(RequestingComponent, Accepted);
	return Launch;
}


void URogueProjectilesSubsystem::PredictProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig,
	AActor* InstigatorActor, FProjectilePredictionKey Key, URogueActionComponent* RequestingComponent)
{
	check(Key.IsValid());
	check(ProjectileConfig);
	if (FindPrediction(Key) || RejectedPredictions.Contains(Key))
	{
		return;
	}

	FProjectileInstance& Instance = ProjectileInstances.AddDefaulted_GetRef();
	Instance.Launch.PredictionKey = Key;
	Instance.Launch.Position = InPosition;
	Instance.Launch.Velocity = InDirection.GetSafeNormal() * ProjectileConfig->InitialSpeed;
	Instance.Launch.ConfigDataAsset = ProjectileConfig;
	Instance.Launch.InstigatorActor = InstigatorActor;
	Instance.Launch.SimulationTime = GetServerTime();
	Instance.Launch.ExpirationTime = GetServerTime() + ProjectileConfig->Lifespan;
	Instance.Position = InPosition;
	Instance.SimulationTime = GetServerTime();
	Instance.RequestingComponent = RequestingComponent;
	StartTracer(Instance);
}


void URogueProjectilesSubsystem::ReceiveLaunch(const FProjectileLaunch& Launch, URogueActionComponent* RequestingComponent)
{
	if (FProjectileUpdate* DeferredImpact = PendingImpacts.Find(Launch.ID))
	{
		if (IsValid(Launch.ConfigDataAsset))
		{
			FProjectileUpdate Resolved = *DeferredImpact;
			Resolved.Projectile.ConfigDataAsset = Launch.ConfigDataAsset;
			ReceiveUpdate(Resolved, RequestingComponent);
		}
	}
	if (Launch.ID <= 0 || CompletedProjectiles.Contains(Launch.ID) || RejectedPredictions.Contains(Launch.PredictionKey))
	{
		return;
	}
	if (!HasSynchronizedTime())
	{
		PendingLaunches.Add(Launch.ID, Launch);
		return;
	}
	if (Launch.ExpirationTime <= GetServerTime())
	{
		ReceiveRemoval(Launch);
		return;
	}
	if (!IsValid(Launch.ConfigDataAsset))
	{
		PendingLaunches.Add(Launch.ID, Launch);
		return;
	}
	PendingLaunches.Remove(Launch.ID);

	FProjectileInstance* Instance = ProjectileInstances.FindByPredicate([&Launch](const FProjectileInstance& Item)
	{
		return Item.Launch.ID == Launch.ID || (Launch.PredictionKey.IsValid() && Item.Launch.PredictionKey == Launch.PredictionKey);
	});
	if (Instance && Instance->Launch.ID == Launch.ID && Instance->Launch.SimulationTime >= Launch.SimulationTime)
	{
		Instance->Launch.InstigatorActor = Launch.InstigatorActor;
		if (RequestingComponent)
		{
			Instance->RequestingComponent = RequestingComponent;
		}
		return;
	}
	const float Now = GetServerTime();
	const float AuthoritativeAge = FMath::Max(0.0f, Now - Launch.SimulationTime);
	const bool bConfirmingPrediction = Instance && Instance->Launch.ID == 0;
	const float PredictedAge = bConfirmingPrediction ? FMath::Max(0.0f, Now - Instance->Launch.SimulationTime) : 0.0f;
	// Accepting the same flight must not replace its local progress with the server's later release time.
	const bool bPreservePredictedFlight = bConfirmingPrediction && PredictedAge >= AuthoritativeAge &&
		Instance->Launch.Position.Equals(Launch.Position, 1.0f) && Instance->Launch.Velocity.Equals(Launch.Velocity, 1.0f);
	if (!Instance)
	{
		Instance = &ProjectileInstances.AddDefaulted_GetRef();
	}

	const bool bKeepTentativeHit = Instance->bTentativeHit && (bConfirmingPrediction || Launch.SimulationTime < Instance->TentativeHitTime);
	const bool bCorrectedCollision = Instance->bTentativeHit && !bKeepTentativeHit;
	Instance->Launch = Launch;
	if (!bPreservePredictedFlight)
	{
		const float Age = FMath::Max(AuthoritativeAge, PredictedAge);
		Instance->Position = Launch.Position + Launch.Velocity * Age;
		Instance->SimulationTime = Now;
	}
	Instance->bTentativeHit = bKeepTentativeHit;
	Instance->bIgnorePredictedCollision |= bCorrectedCollision;
	if (RequestingComponent)
	{
		Instance->RequestingComponent = RequestingComponent;
	}
	if (bKeepTentativeHit)
	{
		StopTracer(*Instance);
	}
	else if (!bPreservePredictedFlight || !Instance->TracerEffectComp)
	{
		StartTracer(*Instance);
	}
}


void URogueProjectilesSubsystem::ReceiveRemoval(const FProjectileLaunch& Launch)
{
	RemoveLocalProjectile(Launch);
	PendingLaunches.Remove(Launch.ID);
	if (!CompletedProjectiles.Contains(Launch.ID))
	{
		FProjectileUpdate Removed;
		Removed.Projectile = Launch;
		Removed.Projectile.ExpirationTime = FMath::Max(Launch.ExpirationTime, GetServerTime() + 2.0f);
		Removed.Outcome = EProjectileOutcome::Expired;
		CompletedProjectiles.Add(Launch.ID, Removed);
	}
}


void URogueProjectilesSubsystem::ReceiveUpdate(const FProjectileUpdate& Update, URogueActionComponent* RequestingComponent)
{
	if (Update.Outcome == EProjectileOutcome::Flying)
	{
		ReceiveLaunch(Update.Projectile, RequestingComponent);
		return;
	}
	RemoveLocalProjectile(Update.Projectile);
	if (Update.Outcome == EProjectileOutcome::Rejected || Update.Outcome == EProjectileOutcome::Cancelled)
	{
		RejectedPredictions.Add(Update.Projectile.PredictionKey, GetServerTime() + 15.0f);
		return;
	}
	ReceiveRemoval(Update.Projectile);
	if (Update.Outcome == EProjectileOutcome::Impacted && !PlayedImpacts.Contains(Update.Projectile.ID))
	{
		if (IsValid(Update.Projectile.ConfigDataAsset))
		{
			PlayedImpacts.Add(Update.Projectile.ID);
			PendingImpacts.Remove(Update.Projectile.ID);
			SpawnImpactFX(Update);
		}
		else
		{
			PendingImpacts.Add(Update.Projectile.ID, Update);
		}
	}
}


void URogueProjectilesSubsystem::RemoveLocalProjectile(const FProjectileLaunch& Launch)
{
	for (int32 Index = ProjectileInstances.Num() - 1; Index >= 0; --Index)
	{
		FProjectileInstance& Instance = ProjectileInstances[Index];
		if ((Launch.ID > 0 && Instance.Launch.ID == Launch.ID) ||
			(Launch.PredictionKey.IsValid() && Instance.Launch.PredictionKey == Launch.PredictionKey))
		{
			StopTracer(Instance);
			ProjectileInstances.RemoveAtSwap(Index);
		}
	}
}


void URogueProjectilesSubsystem::CancelPredictions(AActor* InstigatorActor)
{
	for (int32 Index = ProjectileInstances.Num() - 1; Index >= 0; --Index)
	{
		FProjectileInstance& Instance = ProjectileInstances[Index];
		if (Instance.Launch.ID == 0 && Instance.Launch.InstigatorActor == InstigatorActor)
		{
			StopTracer(Instance);
			ProjectileInstances.RemoveAtSwap(Index);
		}
	}
}


void URogueProjectilesSubsystem::QueryProjectileStatus(int64 ID, URogueActionComponent* RequestingComponent)
{
	check(HasAuthority());
	if (FProjectileInstance* Instance = ProjectileInstances.FindByPredicate([ID](const FProjectileInstance& Item) { return Item.Launch.ID == ID; }))
	{
		if (Instance->RequestingComponent == RequestingComponent && Instance->NextStatusRequestTime <= GetServerTime())
		{
			Instance->NextStatusRequestTime = GetServerTime() + 0.25f;
			FProjectileUpdate Status;
			Status.Projectile = Instance->Launch;
			Status.Projectile.Position = Instance->Position;
			Status.Projectile.SimulationTime = Instance->SimulationTime;
			QueueOwnerUpdate(RequestingComponent, Status);
		}
	}
	else if (const FProjectileUpdate* Completed = CompletedProjectiles.Find(ID))
	{
		if (Completed->Projectile.PredictionKey.Session == RequestingComponent->GetProjectileSession())
		{
			QueueOwnerUpdate(RequestingComponent, *Completed);
		}
	}
}


void URogueProjectilesSubsystem::QueueOwnerUpdate(URogueActionComponent* Owner, const FProjectileUpdate& Update)
{
	if (IsValid(Owner))
	{
		FProjectileOwnerUpdate& Pending = PendingOwnerUpdates.AddDefaulted_GetRef();
		Pending.Owner = Owner;
		Pending.Update = Update;
	}
}


void URogueProjectilesSubsystem::FlushOwnerUpdates()
{
	TArray<FProjectileOwnerUpdate> Updates = MoveTemp(PendingOwnerUpdates);
	TMap<TWeakObjectPtr<URogueActionComponent>, TArray<FProjectileUpdate>> Batches;
	for (const FProjectileOwnerUpdate& Pending : Updates)
	{
		if (Pending.Owner.IsValid())
		{
			Batches.FindOrAdd(Pending.Owner).Add(Pending.Update);
		}
	}
	for (const auto& Batch : Batches)
	{
		for (int32 First = 0; First < Batch.Value.Num(); First += 64)
		{
			TArray<FProjectileUpdate> Part;
			Part.Append(Batch.Value.GetData() + First, FMath::Min(64, Batch.Value.Num() - First));
			Batch.Key->ClientProjectileUpdates(Part);
		}
	}
}


void URogueProjectilesSubsystem::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UpdateProjectiles);
	const float Now = GetServerTime();
	const bool bAuthority = HasAuthority();
	TRACE_COUNTER_SET(LightweightProjectilesCount, ProjectileInstances.Num());

	TArray<FProjectileLaunch> Deferred;
	PendingLaunches.GenerateValueArray(Deferred);
	for (const FProjectileLaunch& Launch : Deferred)
	{
		ReceiveLaunch(Launch);
	}
	for (auto It = CompletedProjectiles.CreateIterator(); It; ++It)
	{
		if (It.Value().Projectile.ExpirationTime + 2.0f < Now)
		{
			PlayedImpacts.Remove(It.Key());
			PendingImpacts.Remove(It.Key());
			It.RemoveCurrent();
		}
	}
	for (auto It = RejectedPredictions.CreateIterator(); It; ++It)
	{
		if (It.Value() < Now)
		{
			It.RemoveCurrent();
		}
	}

	TArray<TPair<int64, FHitResult>> Hits;
	TArray<int64> Expired;
	for (int32 Index = ProjectileInstances.Num() - 1; Index >= 0; --Index)
	{
		FProjectileInstance& Instance = ProjectileInstances[Index];
		if (Instance.Launch.ExpirationTime <= Now)
		{
			if (bAuthority)
			{
				Expired.Add(Instance.Launch.ID);
			}
			else
			{
				StopTracer(Instance);
				ProjectileInstances.RemoveAtSwap(Index);
			}
			continue;
		}
		if (Instance.bTentativeHit)
		{
			if (Instance.Launch.ID > 0 && Instance.RequestingComponent.IsValid() && Instance.NextStatusRequestTime <= Now)
			{
				Instance.NextStatusRequestTime = Now + 0.5f;
				Instance.RequestingComponent->ServerQueryProjectileStatus(Instance.Launch.ID);
			}
			continue;
		}

		const float Step = FMath::Max(0.0f, Now - Instance.SimulationTime);
		const FVector NextPosition = Instance.Position + Instance.Launch.Velocity * Step;
		// Remote observers render authority data; only the shooter predicts collisions.
		if (bAuthority || (Instance.RequestingComponent.IsValid() && !Instance.bIgnorePredictedCollision))
		{
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(Instance.Launch.InstigatorActor);
			TArray<FHitResult> HitResults;
			GetWorld()->SweepMultiByChannel(HitResults, Instance.Position, NextPosition, FQuat::Identity,
				COLLISION_PROJECTILE, FCollisionShape::MakeSphere(20.0f), Params);
			for (const FHitResult& Hit : HitResults)
			{
				AActor* HitActor = Hit.GetActor();
				if (!Hit.bBlockingHit && (!IsValid(HitActor) || !HitActor->CanBeDamaged()))
				{
					continue;
				}
				if (IsValid(HitActor))
				{
					if (const IGenericTeamAgentInterface* Team = Cast<IGenericTeamAgentInterface>(Instance.Launch.InstigatorActor))
					{
						if (Team->GetTeamAttitudeTowards(*HitActor) == ETeamAttitude::Friendly)
						{
							continue;
						}
					}
				}
				Instance.Position = Hit.Location;
				if (bAuthority)
				{
					Hits.Emplace(Instance.Launch.ID, Hit);
				}
				else
				{
					Instance.bTentativeHit = true;
					Instance.TentativeHitTime = Now;
					StopTracer(Instance);
				}
				break;
			}
		}
		if (!Instance.bTentativeHit)
		{
			Instance.Position = NextPosition;
			Instance.SimulationTime = Now;
		}
	}
	for (const auto& Hit : Hits)
	{
		FinishProjectile(Hit.Key, &Hit.Value);
	}
	for (int64 ID : Expired)
	{
		FinishProjectile(ID, nullptr);
	}
	if (!PendingObserverUpdates.IsEmpty())
	{
		TArray<FProjectileUpdate> Updates = MoveTemp(PendingObserverUpdates);
		for (int32 First = 0; First < Updates.Num(); First += 64)
		{
			TArray<FProjectileUpdate> Part;
			Part.Append(Updates.GetData() + First, FMath::Min(64, Updates.Num() - First));
			GetWorld()->GetGameState<ARogueGameState>()->MulticastProjectileImpacts(Part);
		}
	}
	FlushOwnerUpdates();
}


void URogueProjectilesSubsystem::FinishProjectile(int64 ID, const FHitResult* Hit)
{
	FProjectileInstance* Instance = ProjectileInstances.FindByPredicate([ID](const FProjectileInstance& Item) { return Item.Launch.ID == ID; });
	if (!Instance)
	{
		return;
	}
	FProjectileUpdate Update;
	Update.Projectile = Instance->Launch;
	Update.Projectile.Position = Instance->Position;
	Update.Projectile.SimulationTime = GetServerTime();
	Update.Outcome = Hit ? EProjectileOutcome::Impacted : EProjectileOutcome::Expired;
	if (Hit)
	{
		Update.Projectile.Position = Hit->Location;
		Update.ImpactPosition = Hit->ImpactPoint;
		Update.ImpactNormal = Hit->ImpactNormal;
		const UPrimitiveComponent* HitComp = Hit->GetComponent();
		Update.bSpawnDecal = HitComp && HitComp->bReceivesDecals && HitComp->GetMobility() != EComponentMobility::Movable;
	}
	QueueOwnerUpdate(Instance->RequestingComponent.Get(), Update);
	CompletedProjectiles.Add(ID, Update);
	// Damage deliberately remains disabled for this experimental path.
	RemoveLocalProjectile(Update.Projectile);

	ARogueGameState* GS = GetWorld()->GetGameState<ARogueGameState>();
	check(GS);
	GS->ProjectileData.Items.RemoveAllSwap([ID](const FProjectileItem& Item) { return Item.Projectile.ID == ID; });
	GS->ProjectileData.MarkArrayDirty();
	GS->ForceNetUpdate();
	if (Hit)
	{
		PendingObserverUpdates.Add(Update);
	}
}


void URogueProjectilesSubsystem::StartTracer(FProjectileInstance& Instance)
{
	if (GetWorld()->GetNetMode() != NM_DedicatedServer && Instance.Launch.ConfigDataAsset->ProjectileEffect)
	{
		if (Instance.TracerEffectComp)
		{
			Instance.TracerEffectComp->SetWorldLocationAndRotation(Instance.Position, Instance.Launch.Velocity.Rotation());
			Instance.TracerEffectComp->ReinitializeSystem();
		}
		else
		{
			Instance.TracerEffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Instance.Launch.ConfigDataAsset->ProjectileEffect,
				Instance.Position, Instance.Launch.Velocity.Rotation(), FVector(1), true, true, ENCPoolMethod::AutoRelease);
		}
		if (Instance.TracerEffectComp)
		{
			// Preserve the existing world-space emitter contract; anchor a corrected tracer at its new simulation position.
			Instance.TracerEffectComp->SetVariablePosition(TEXT("ProjectileVelocity"), FVector::ForwardVector * Instance.Launch.Velocity.Size());
		}
	}
}


void URogueProjectilesSubsystem::StopTracer(FProjectileInstance& Instance)
{
	if (Instance.TracerEffectComp)
	{
		Instance.TracerEffectComp->Deactivate();
		Instance.TracerEffectComp = nullptr;
	}
}


void URogueProjectilesSubsystem::SpawnImpactFX(const FProjectileUpdate& Update)
{
	++ImpactPlaybackCount;
	if (GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	const URogueProjectileData* Config = Update.Projectile.ConfigDataAsset;
	if (Config->ImpactEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Config->ImpactEffect, Update.ImpactPosition,
			Update.Projectile.Velocity.Rotation(), FVector(1), true, true, ENCPoolMethod::AutoRelease);
	}
	if (Update.bSpawnDecal && Config->ImpactDecal_DataChannel)
	{
		const UNiagaraDataChannel* NDC = Config->ImpactDecal_DataChannel->Get();
		FNDCAccessContextInst AccessContext = NDC->GetTransientAccessContext();
		FNDCAccessContextLegacy& Context = AccessContext.GetChecked<FNDCAccessContextLegacy>();
		Context.Location = Update.ImpactPosition;
		UNiagaraDataChannelWriter* Writer = UNiagaraDataChannelLibrary::WriteToNiagaraDataChannel_WithContext(GetWorld(),
			Config->ImpactDecal_DataChannel, AccessContext, 1, false, true, false, TEXT("ImpactDecals"));
		if (Writer)
		{
			Writer->WriteVector(TEXT("ImpactLocation"), 0, Update.ImpactPosition);
			Writer->WriteVector(TEXT("ImpactNormal"), 0, Update.ImpactNormal);
		}
	}
}


const FProjectileInstance* URogueProjectilesSubsystem::FindProjectile(int64 ID) const
{
	return ProjectileInstances.FindByPredicate([ID](const FProjectileInstance& Item) { return Item.Launch.ID == ID; });
}


const FProjectileInstance* URogueProjectilesSubsystem::FindPrediction(FProjectilePredictionKey Key) const
{
	return ProjectileInstances.FindByPredicate([Key](const FProjectileInstance& Item) { return Item.Launch.PredictionKey == Key; });
}


float URogueProjectilesSubsystem::GetServerTime() const
{
	if (const AGameStateBase* GS = GetWorld()->GetGameState())
	{
		return GS->GetServerWorldTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds();
}


void URogueProjectilesSubsystem::Deinitialize()
{
	for (FProjectileInstance& Instance : ProjectileInstances)
	{
		StopTracer(Instance);
	}
	ProjectileInstances.Reset();
	PendingLaunches.Reset();
	PendingImpacts.Reset();
	CompletedProjectiles.Reset();
	PendingOwnerUpdates.Reset();
	PendingObserverUpdates.Reset();
	Super::Deinitialize();
}


TStatId URogueProjectilesSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueProjectilesSubsystem, STATGROUP_Tickables);
}


bool URogueProjectilesSubsystem::HasAuthority() const
{
	return GetWorld()->GetNetMode() != NM_Client;
}
