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
	CurrInstanceID++;
	InternalCreateProjectile(InPosition, InDirection, ProjectileConfig, InstigatorActor, CurrInstanceID);
}

// @todo: rename as this is used 'outside' this class too
void URogueProjectilesSubsystem::InternalCreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor, uint32 NewID)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CreateProjectile);

	// Active data that represents the moving projectile
	FProjectileInstance ProjInfo = FProjectileInstance(InPosition, InDirection * ProjectileConfig->InitialSpeed, NewID);
	ProjectileInstances.Add(ProjInfo);

	UWorld* World = GetWorld();
		
	UNiagaraComponent* EffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ProjectileConfig->ProjectileEffect, InPosition,
		InDirection.Rotation(), FVector(1), true, true, ENCPoolMethod::AutoRelease);
	
	// The component stays in world space, the emitters themselves move along the axis of the projectile to match
	// the collision query locations
	const FName UserParamName = "ProjectileVelocity";
	FVector LocalVelocity = FVector::ForwardVector * ProjectileConfig->InitialSpeed;
	EffectComp->SetVariablePosition(UserParamName, LocalVelocity);

	// Now assign the FX Comp to the ProjectileConfig already in the local array
	FProjectileConfigArray& DataArray = World->GetGameState<ARogueGameState>()->ProjectileData;

	if (HasAuthority())
	{
		float ExpirationGameTime = World->TimeSeconds + ProjectileConfig->Lifespan;
		// Full data of the projectile instance, used for bookkeeping and replication, not constantly updated
		FProjectileConfig Info = FProjectileConfig(InPosition, InDirection, ProjectileConfig, InstigatorActor, NewID, ExpirationGameTime);
	
		World->GetGameState<ARogueGameState>()->ProjectileData.Items.Add(Info);
		World->GetGameState<ARogueGameState>()->ProjectileData.MarkItemDirty(Info);
	}
	
	FProjectileConfig* Data = DataArray.Items.FindByPredicate([NewID](FProjectileConfig& Item){ return NewID == Item.ID;  });
	Data->TracerEffectComp = EffectComp;

	// Start Pos
	DrawDebugBox(World, InPosition, FVector(15.0f), FColor::Green, false, 2.0f);
}


void URogueProjectilesSubsystem::RemoveProjectileID(uint32 IdToRemove)
{
	UWorld* World = GetWorld();

	FProjectileConfigArray& DataArray = World->GetGameState<ARogueGameState>()->ProjectileData;

	// @todo: faster lookup available like a local TMap cache
	FProjectileConfig& ProjConfig = *DataArray.Items.FindByPredicate([IdToRemove](FProjectileConfig& Item){ return IdToRemove == Item.ID;  });
	
	if (UNiagaraComponent* FXComp = ProjConfig.TracerEffectComp)
	{
		FXComp->Deactivate();
		FXComp = nullptr;
	}

	if (ProjConfig.Hit.GetActor() && !ProjConfig.bHasPlayedImpact)
	{
		ProjConfig.bHasPlayedImpact = true;
			
		FRotator ImpactRotation = (ProjConfig.Hit.TraceEnd - ProjConfig.Hit.TraceStart).GetSafeNormal().Rotation();
		SpawnImpactFX(World, ProjConfig, ProjConfig.Hit.Location, ImpactRotation);
	}
	
	//DrawDebugBox(World, ProjConfig.Hit.Location, FVector(40.0f), FColor::Red, false, 2.0f);

	FProjectileInstance TempInst = FProjectileInstance(IdToRemove);
	// Perhaps hacky to use a temp struct to 'find' it by ID in the array
	ProjectileInstances.Remove(TempInst);
}


void URogueProjectilesSubsystem::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UpdateProjectiles);

	const UWorld* World = GetWorld();

	ECollisionChannel CollisionChannel = COLLISION_PROJECTILE;

	FCollisionShape Shape;
	Shape.SetSphere(20.0f);

	// Track projectiles to end this frame for batched update
	TArray<int32> ProjectileHitIDs;

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
			FProjectileConfig& ProjConfig = *ProjectileConfigs.Items.FindByPredicate([Proj](const FProjectileConfig& OtherProj){ return Proj.ID == OtherProj.ID;  });

			// Where we want to end up "next frame" (once we move to async queries)
			FVector NextPosition = Proj.Position + (Proj.Velocity * DeltaTime);
			
			TArray<FHitResult> HitResults;		
			// True only blocking hit
			if (World->SweepMultiByChannel(HitResults, Proj.Position, NextPosition, FQuat::Identity, CollisionChannel, Shape))
			{
				// As far as we can move till hit
				Proj.Position = HitResults.Last().Location;
		
				//DrawDebugSphere(World, Proj.Position, Shape.GetSphereRadius(), 20, FColor::Green, false, 2.0f);
		
				// (Blocking) Hit!
				ProjConfig.Hit = HitResults[0];
				
				ProjectileHitIDs.Add(Proj.ID);

				// Push hit to clients
				// @todo: is auth check required?
				if (HasAuthority())
				{
					// Give a bit of time before deletion for clients to rep the Hit
					ProjConfig.ExpirationGameTime = (World->TimeSeconds + 1.0);
					
					ProjectileConfigs.MarkItemDirty(ProjConfig);
				}
				continue;
			}
			// Any overlaps?
			for (int32 HitIndex = 0; HitIndex < HitResults.Num(); HitIndex++)
			{
				FHitResult& Hit = HitResults[HitIndex];
				AActor* HitActor = Hit.GetActor();
			
				if (HitActor->CanBeDamaged())
				{
					if (ProjConfig.InstigatorActor == HitActor)
					{
						// Skip the owner of the bullet
						continue;
					}

					// Check for friendly fire, eg. skip hitting other AI agents
					if (IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(ProjConfig.InstigatorActor))
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
					// at this point we know it can be damaged and if it has TeamInterface, it will not be "friendly" to projectile instigator
					ProjConfig.Hit = Hit;
					
					ProjectileHitIDs.AddUnique(Proj.ID);

					// Push hit to clients
					// @todo: is auth check required?
					if (HasAuthority())
					{
						// Give a bit of time before deletion for clients to rep the Hit
						ProjConfig.ExpirationGameTime = (World->TimeSeconds + 1.0);
						
						ProjectileConfigs.MarkItemDirty(ProjConfig);
					}
					break;
				}
			}
			
			// no hit found, Move forward
			// for overlaps, the impactPosition should be used for things like VFX playback instead of the projectile location
			Proj.Position = NextPosition;
		}
	}
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ProcessProjectileHits);

		for (int i = ProjectileHitIDs.Num() - 1; i >= 0; --i)
		{
			int32 HitID = ProjectileHitIDs[i];
			
			FProjectileConfig& ProjConfig = *ProjectileConfigs.Items.FindByPredicate([HitID](const FProjectileConfig& OtherProj){ return HitID == OtherProj.ID;  });

			// Apply Damage (server-only, misc. stuff like impulses still runs on clients too)
			// @todo-fixme: disabled damage to make things much easier to test
			/*FGameplayTagContainer ContextTags;
			URogueGameplayFunctionLibrary::ApplyDirectionalDamage(ProjConfig.InstigatorActor, ProjConfig.Hit.GetActor(),
				ProjConfig.ConfigDataAsset->DamageCoefficient, ProjConfig.Hit, ContextTags);*/

			// Will play any impact vfx during removal
			RemoveProjectileID(HitID);
		}
	}

	if (HasAuthority())
	{
		// Cleanup of old projectiles
		float CurrGameTime = GetWorld()->TimeSeconds;
		for (int Index = ProjectileConfigs.Items.Num() - 1; Index >= 0; --Index)
		{
			FProjectileConfig& Item = ProjectileConfigs.Items[Index];
			// Is the projectile older than MaxLifespan seconds into the past
			if (Item.ExpirationGameTime < CurrGameTime)
			{
				RemoveProjectileID(Item.ID);

				// The only place we actually delete the data, we expire projectiles slightly after a hit/overlap to give replication some time to finish
				ProjectileConfigs.Items.RemoveSingle(Item);
				ProjectileConfigs.MarkArrayDirty();
			}
		}
	}
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

bool URogueProjectilesSubsystem::HasAuthority() const
{
	const UWorld* World = GetWorld();
	return World->GetNetMode() != (NM_Client);
}
