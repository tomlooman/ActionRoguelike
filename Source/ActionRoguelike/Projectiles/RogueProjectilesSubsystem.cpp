// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectilesSubsystem.h"

#include "ActionRoguelike.h"
#include "GenericTeamAgentInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannelAccessor.h"
#include "NiagaraFunctionLibrary.h"
#include "RogueProjectileData.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "Core/RogueGameState.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(LightweightProjectilesCount, TEXT("Game/DataOnlyProjectileCount"));


void URogueProjectilesSubsystem::CreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CreateProjectile);
	
	UWorld* World = GetWorld();
	
	CurrInstanceID++;

	InternalCreateProjectile(InPosition, InDirection, ProjectileConfig, InstigatorActor, CurrInstanceID);

	// @todo: lifetime tracking can be server-only
	Lifetimes.Add(GetWorld()->TimeSeconds);
}

// @todo: rename as this is used 'outside' this class too
void URogueProjectilesSubsystem::InternalCreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor, uint32 NewID)
{
	// Active data that represents the moving projectile
	FProjectileInstance ProjInfo = FProjectileInstance(InPosition, InDirection * ProjectileConfig->InitialSpeed, CurrInstanceID);
	ProjectileInstances.Add(ProjInfo);
		
	UNiagaraComponent* EffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ProjectileConfig->ProjectileEffect, InPosition,
		InDirection.Rotation(), FVector(1), true, true, ENCPoolMethod::AutoRelease);
	// The component stays in world space, the emitters themselves move along the axis of the projectile to match
	// the collision query locations
	const FName UserParamName = "ProjectileVelocity";
	FVector LocalVelocity = FVector::ForwardVector * ProjectileConfig->InitialSpeed;
	EffectComp->SetVariablePosition(UserParamName, LocalVelocity);

	// Now assign the FX Comp to the ProjectileConfig already in the local array
	UWorld* World = GetWorld();
	FProjectileConfigArray& DataArray = World->GetGameState<ARogueGameState>()->ProjectileData;

	if (InstigatorActor->HasAuthority())
	{
		// Full data of the projectile instance, used for bookkeeping and replication, not constantly updated
		FProjectileConfig Info = FProjectileConfig(InPosition, InDirection, ProjectileConfig, InstigatorActor, CurrInstanceID);
	
		World->GetGameState<ARogueGameState>()->ProjectileData.Items.Add(Info);
		World->GetGameState<ARogueGameState>()->ProjectileData.MarkItemDirty(Info);
	}
	
	FProjectileConfig* Data = DataArray.Items.FindByPredicate([NewID](FProjectileConfig& Item){ return NewID == Item.ID;  });
	Data->TracerEffectComp = EffectComp;

	// Start Pos
	DrawDebugBox(World, InPosition, FVector(15.0f), FColor::Green, false, 2.0f);
}


/*void URogueProjectilesSubsystem::RemoveProjectile(int32 Index)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(RemoveProjectiles);

	UWorld* World = GetWorld();

	FProjectileConfigArray& DataArray = World->GetGameState<ARogueGameState>()->ProjectileData;

	FProjectileConfig& Data = DataArray.Items[Index];
	if (UNiagaraComponent* FXComp = Data.TracerEffectComp)
	{
		FXComp->Deactivate();
	}

	DataArray.Items.RemoveAt(Index, EAllowShrinking::No);
	DataArray.MarkArrayDirty();
	
	ProjectileInstances.RemoveAt(Index, EAllowShrinking::No);
	
	Lifetimes.RemoveAt(Index, EAllowShrinking::No);
}*/


void URogueProjectilesSubsystem::RemoveProjectileID(uint32 IdToRemove)
{
	// @todo: replace all uses of RemoveProjectile() variant above.
	UWorld* World = GetWorld();

	FProjectileConfigArray& DataArray = World->GetGameState<ARogueGameState>()->ProjectileData;

	FProjectileConfig& Data = *DataArray.Items.FindByPredicate([IdToRemove](FProjectileConfig& Item){ return IdToRemove == Item.ID;  });

	//FProjectileConfig& Data = DataArray.Items[Index];
	if (UNiagaraComponent* FXComp = Data.TracerEffectComp)
	{
		FXComp->Deactivate();
	}

	if (Data.InstigatorActor->HasAuthority())
	{
		DataArray.Items.RemoveSingle(Data);
		DataArray.MarkArrayDirty();
	}

	// Hit/Overlap Pos(?) @todo: 0,0,0 on clients, its NOT filled because clients SKIP collision checks,
	// @todo: CHANGE CLIENTS TO ALSO RUN HIT TESTS AND ONLY SKIP DMG APPLY
	DrawDebugBox(World, Data.Hit.Location, FVector(40.0f), FColor::Red, false, 2.0f);
	
	// Perhaps hacky to use a temp struct to 'find' it by ID in the array
	ProjectileInstances.Remove(FProjectileInstance(IdToRemove));

	// @todo: fix the lifetime stuff too
	//Lifetimes.RemoveAt(Index, EAllowShrinking::No);
}


void URogueProjectilesSubsystem::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UpdateProjectiles);

	const UWorld* World = GetWorld();

	ECollisionChannel CollisionChannel = COLLISION_PROJECTILE;

	FCollisionShape Shape;
	Shape.SetSphere(20.0f);

	// Track projectiles to end this frame for batched update
	TArray<int32> ProjectileHits;

	ARogueGameState* GS = World->GetGameState<ARogueGameState>();
	if (GS == nullptr)
	{
		// While joining we may not have the gamestate yet on clients
		return;
	}

	FProjectileConfigArray& ProjectileConfigs = World->GetGameState<ARogueGameState>()->ProjectileData;

	TRACE_COUNTER_SET(LightweightProjectilesCount, ProjectileInstances.Num());
	
	// @TODO & Notes
	// We could cache a collision params to ignore instigator during the sweep, now I dont want to fetch instigator from the metadata every time
	//		so instead we end up only filtering against this when any overlap is found, which currently works well enough
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoveProjectiles);
		
		for (int32 ProjIndex = 0; ProjIndex < ProjectileInstances.Num(); ProjIndex++)
		{
			FProjectileInstance& Proj = ProjectileInstances[ProjIndex];

			// Where we want to end up "next frame" (once we move to async queries)
			FVector NextPosition = Proj.Position + (Proj.Velocity * DeltaTime);

			if (GS->HasAuthority())
			{
				TArray<FHitResult> HitResults;		
				// True only blocking hit
				if (World->SweepMultiByChannel(HitResults, Proj.Position, NextPosition, FQuat::Identity, CollisionChannel, Shape))
				{
					// As far as we can move till hit
					Proj.Position = HitResults.Last().Location;
			
					//DrawDebugSphere(World, Proj.Position, Shape.GetSphereRadius(), 20, FColor::Green, false, 2.0f);
			
					// (Blocking) Hit!
					ProjectileConfigs.Items[ProjIndex].Hit = HitResults[0];
					ProjectileHits.Add(ProjIndex);
					continue;
				}
				// Any overlaps?
				for (int32 HitIndex = 0; HitIndex < HitResults.Num(); HitIndex++)
				{
					FHitResult& Hit = HitResults[HitIndex];
					AActor* HitActor = Hit.GetActor();
				
					if (HitActor->CanBeDamaged())
					{
						FProjectileConfig& MetaData = ProjectileConfigs.Items[ProjIndex];
						if (MetaData.InstigatorActor == HitActor)
						{
							// Skip the owner of the bullet
							continue;
						}

						// Check for friendly fire, eg. skip hitting other AI agents
						if (IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(MetaData.InstigatorActor))
						{
							ETeamAttitude::Type Attitude = TeamInterface->GetTeamAttitudeTowards(*HitActor);
							if (Attitude == ETeamAttitude::Friendly)
							{
								// Show we ignored a friendly unit
								//DrawDebugBox(World, Hit.Location, FVector(40.0f), FColor::Red, false, 2.0f);
								continue;
							}
						}

						// We only use the first non-friendly overlap as our expected hit
						// at this point we know it can be damaged and if it has TeamInterface, it will not be "friendly" to projectile instigator)
						ProjectileHits.AddUnique(ProjIndex);
						ProjectileConfigs.Items[ProjIndex].Hit = Hit;
						break;
					}
				}
			}

			// no hit found, Move forward
			// for overlaps, the impactPosition should be used for things like VFX playback instead of the projectile location
			Proj.Position = NextPosition;
		}
	}
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ProcessProjectileHits);

		for (int i = ProjectileHits.Num() - 1; i >= 0; --i)
		{
			int32 HitIndex = ProjectileHits[i];
			FProjectileConfig& BulkData = ProjectileConfigs.Items[HitIndex];

			// Apply Damage
			FGameplayTagContainer ContextTags;
			URogueGameplayFunctionLibrary::ApplyDirectionalDamage(BulkData.InstigatorActor, BulkData.Hit.GetActor(),
				BulkData.ConfigDataAsset->DamageCoefficient, BulkData.Hit, ContextTags);

			FVector Position = ProjectileInstances[HitIndex].Position;
			// Inverted normalized velocity should be a good orientation for any directional impact vfx
			FRotator ImpactRotation = ProjectileInstances[HitIndex].Velocity.GetSafeNormal().Rotation().GetInverse();

			SpawnImpactFX(World, BulkData, Position, ImpactRotation);

			//RemoveProjectile(HitIndex);
			RemoveProjectileID(ProjectileInstances[HitIndex].ID);
		}
	}

	// Cleanup of old projectiles
	const float MaxLifespan = 20.0f;
	float ExpirationGameTime = GetWorld()->TimeSeconds - MaxLifespan;
	for (int Index = Lifetimes.Num() - 1; Index >= 0; --Index)
	{
		// Is the projectile older than MaxLifespan seconds into the past
		if (Lifetimes[Index] < (ExpirationGameTime))
		{
			//RemoveProjectile(Index);
			RemoveProjectileID(ProjectileInstances[Index].ID);
		}
	}

	// Batch "deactivate" all the removed projectiles
	// This might just be rolled into the RemoveProjectile
	// @todo: make server-only
	/*for (int i = 0; i < RemovedProjectiles.Num(); ++i)
	{
		FProjectileFullData& Data = RemovedProjectiles[i];
		if (UNiagaraComponent* FXComp = Data.FXComp)
		{
			FXComp->Deactivate();
		}
		// handle any other deactivation such as looped sound fx.
		// ...
	}
	RemovedProjectiles.Reset();*/
}


void URogueProjectilesSubsystem::SpawnImpactFX(const UWorld* World, const FProjectileConfig& ProjConfig, FVector ImpactPosition, FRotator ImpactRotation)
{
	// Impact Explosion
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ProjConfig.ConfigDataAsset->ImpactEffect, ImpactPosition, ImpactRotation,
		FVector(1), true, true, ENCPoolMethod::AutoRelease, true);

	// Helps find the correct island to inject this particle into
	FNiagaraDataChannelSearchParameters Params;
	Params.Location = ImpactPosition;

	// Intended for DECALs, using the Data Channels rather than relying on individual particle systems
	UNiagaraDataChannelWriter* Writer = UNiagaraDataChannelLibrary::WriteToNiagaraDataChannel(World, ProjConfig.ConfigDataAsset->ImpactDecal_DataChannel,
		Params, 1, false, true, true, "ImpactDecals");

	Writer->WriteVector("ImpactLocation", 0, ImpactPosition);
	Writer->WriteVector("ImpactNormal", 0, ImpactRotation.Vector());
}


TStatId URogueProjectilesSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueProjectilesSubsystem, STATGROUP_Tickables);
}