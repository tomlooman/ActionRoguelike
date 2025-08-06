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
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(LightweightProjectilesCount, TEXT("Game/DataOnlyProjectileCount"));


void URogueProjectilesSubsystem::CreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CreateProjectile);
	
	UWorld* World = GetWorld();
	
	FProjectileInfo ProjInfo;
	ProjInfo.Position = InPosition;
	ProjInfo.Velocity = InDirection * ProjectileConfig->InitialSpeed;

	ProjectileData.Add(ProjInfo);

	// @todo: verify pooling works
	UNiagaraComponent* EffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ProjectileConfig->ProjectileEffect, InPosition,
		InDirection.Rotation(), FVector(1), true, true, ENCPoolMethod::AutoRelease);

	// The component stays in world space, the emitters themselves move along the axis of the projectile to match
	// the collision query locations
	const FName UserParamName = "ProjectileVelocity";
	FVector LocalVelocity = FVector::ForwardVector * ProjectileConfig->InitialSpeed;
	EffectComp->SetVariablePosition(UserParamName, LocalVelocity);
	
	FProjectileFullData Info;
	Info.FXComp = EffectComp;
	Info.ConfigData = ProjectileConfig;
	Info.InstigatorActor = InstigatorActor;
	ProjectileMetaData.Add(Info);
	
	Lifetimes.Add(GetWorld()->TimeSeconds);
}


void URogueProjectilesSubsystem::RemoveProjectile(int32 Index)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(RemoveProjectiles);

	RemovedProjectiles.Add(ProjectileMetaData[Index]);
		
	// RemoveAtSwap works while we immediately remove the projectile rather than wait till the end of frame
	// We must use reverse for-loops to avoid running anything twice on the projectile that was swapped from the end of the array
	ProjectileData.RemoveAtSwap(Index, EAllowShrinking::No);
	ProjectileMetaData.RemoveAtSwap(Index, EAllowShrinking::No);
	Lifetimes.RemoveAtSwap(Index, EAllowShrinking::No);
}


void URogueProjectilesSubsystem::Tick(float DeltaTime)
{
	TRACE_COUNTER_SET(LightweightProjectilesCount, ProjectileData.Num());
	TRACE_CPUPROFILER_EVENT_SCOPE(UpdateProjectiles);

	const UWorld* World = GetWorld();

	ECollisionChannel CollisionChannel = COLLISION_PROJECTILE;

	FCollisionShape Shape;
	Shape.SetSphere(20.0f);

	// Track projectiles to end this frame for batched update
	TArray<int32> ProjectileHits;

	// @TODO & Notes
	// We could cache a collision params to ignore instigator during the sweep, now I dont want to fetch instigator from the metadata every time
	//		so instead we end up only filtering against this when any overlap is found, which currently works well enough
	
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(MoveProjectiles);
		
		for (int32 ProjIndex = 0; ProjIndex < ProjectileData.Num(); ProjIndex++)
		{
			FProjectileInfo& Proj = ProjectileData[ProjIndex];

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
				ProjectileMetaData[ProjIndex].Hit = HitResults[0];
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
					FProjectileFullData& MetaData = ProjectileMetaData[ProjIndex];
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
					ProjectileMetaData[ProjIndex].Hit = Hit;
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

		for (int i = ProjectileHits.Num() - 1; i >= 0; --i)
		{
			int32 HitIndex = ProjectileHits[i];
			FProjectileFullData& BulkData = ProjectileMetaData[HitIndex];

			// Apply Damage
			FGameplayTagContainer ContextTags;
			URogueGameplayFunctionLibrary::ApplyDirectionalDamage(BulkData.InstigatorActor, BulkData.Hit.GetActor(),
				BulkData.ConfigData->DamageCoefficient, BulkData.Hit, ContextTags);

			FVector Position = ProjectileData[HitIndex].Position;
			// Inverted normalized velocity should be a good orientation for any directional impact vfx
			FRotator ImpactRotation = ProjectileData[HitIndex].Velocity.GetSafeNormal().Rotation().GetInverse();

			SpawnImpactFX(World, BulkData, Position, ImpactRotation);

			RemoveProjectile(HitIndex);
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
			RemoveProjectile(Index);
		}
	}

	// Batch "deactivate" all the removed projectiles
	// This might just be rolled into the RemoveProjectile
	for (int i = 0; i < RemovedProjectiles.Num(); ++i)
	{
		FProjectileFullData& Data = RemovedProjectiles[i];
		if (UNiagaraComponent* FXComp = Data.FXComp)
		{
			FXComp->Deactivate();
		}
		// handle any other deactivation such as looped sound fx.
		// ...
	}
	RemovedProjectiles.Reset();
}


void URogueProjectilesSubsystem::SpawnImpactFX(const UWorld* World, const FProjectileFullData& BulkData, FVector ImpactPosition, FRotator ImpactRotation)
{
	// Impact Explosion
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BulkData.ConfigData->ImpactEffect, ImpactPosition, ImpactRotation,
		FVector(1), true, true, ENCPoolMethod::AutoRelease, true);

	// Helps find the correct island to inject this particle into
	FNiagaraDataChannelSearchParameters Params;
	Params.Location = ImpactPosition;

	// Intended for DECALs, using the Data Channels rather than relying on individual particle systems
	UNiagaraDataChannelWriter* Writer = UNiagaraDataChannelLibrary::WriteToNiagaraDataChannel(World, BulkData.ConfigData->ImpactDecal_DataChannel,
		Params, 1, false, true, true, "ImpactDecals");

	Writer->WriteVector("ImpactLocation", 0, ImpactPosition);
	Writer->WriteVector("ImpactNormal", 0, ImpactRotation.Vector());
}


TStatId URogueProjectilesSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueProjectilesSubsystem, STATGROUP_Tickables);
}
