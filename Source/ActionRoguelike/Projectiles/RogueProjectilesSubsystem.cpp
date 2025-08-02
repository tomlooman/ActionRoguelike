// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectilesSubsystem.h"

#include "ActionRoguelike.h"
#include "GenericTeamAgentInterface.h"
#include "NiagaraComponent.h"
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
	
	ProjectileData.RemoveAt(Index, EAllowShrinking::No);
	ProjectileMetaData.RemoveAt(Index, EAllowShrinking::No);
	Lifetimes.RemoveAt(Index, EAllowShrinking::No);
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
	TArray<int32> ExpiredIndices;

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
			
				// Hit!
				ProjectileMetaData[ProjIndex].Hit = HitResults[0];
				ProjectileHits.Add(ProjIndex);
				continue;
			}
			// Any overlaps?
			if (HitResults.Num() > 0)
			{
				for (int32 HitIndex = 0; HitIndex < HitResults.Num(); HitIndex++)
				{
					FHitResult& Hit = HitResults[HitIndex];
				
					if (Hit.GetActor()->CanBeDamaged())
					{
						FProjectileFullData& MetaData = ProjectileMetaData[ProjIndex];
						if (MetaData.InstigatorActor == Hit.GetActor())
						{
							// Skip the owner of the bullet
							continue;
						}

						// Check for friendly fire, eg. skip hitting other AI agents
						if (IGenericTeamAgentInterface* TeamInterface = Cast<IGenericTeamAgentInterface>(MetaData.InstigatorActor))
						{
							ETeamAttitude::Type Attitude = TeamInterface->GetTeamAttitudeTowards(*Hit.GetActor());
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
			}

			// no hit found, Move forward
			Proj.Position = NextPosition;
		}
	}
	
	// Debug only
	/*for (FProjectileInfo& Proj : ProjectileData)
	{
		DrawDebugPoint(World, Proj.Position, 10.0f, FColor::Orange, false, 1.0f);
	}*/

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ProcessProjectileHits);
		
		for (int32 Index : ProjectileHits)
		{
			FProjectileFullData& BulkData = ProjectileMetaData[Index];

			// Disable Projectile Visuals
			BulkData.FXComp->Deactivate();

			// Apply Damage
			FGameplayTagContainer ContextTags;
			URogueGameplayFunctionLibrary::ApplyDirectionalDamage(BulkData.InstigatorActor, BulkData.Hit.GetActor(),
				BulkData.ConfigData->DamageCoefficient, BulkData.Hit, ContextTags);

			FVector Position = ProjectileData[Index].Position;
			// Inverted normalized velocity should be a good orientation for any directional impact vfx
			FRotator ImpactRotation = ProjectileData[Index].Velocity.GetSafeNormal().Rotation().GetInverse();

			// Impact VFX
			// @todo: setup pre-culling in the particle systems to prevent spawning when not rendered
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, BulkData.ConfigData->ImpactEffect, Position, ImpactRotation,
				FVector(1), true, true, ENCPoolMethod::AutoRelease, true);

			// Mark for cleanup
			ExpiredIndices.Add(Index);
		}
	}

	const float MaxLifespan = 20.0f;
	float ExpirationGameTime = GetWorld()->TimeSeconds - MaxLifespan;
	// Cleanup of old projectiles
	for (int32 i = 0; i < Lifetimes.Num(); i++)
	{
		// Is the projectile older than MaxLifespan seconds into the past
		if (Lifetimes[i] < (ExpirationGameTime))
		{
			ExpiredIndices.AddUnique(i);
		}
	}

	// Cleanup all projectiles marked for deletion
	for (int32 i = 0; i < ExpiredIndices.Num(); i++)
	{
		RemoveProjectile(ExpiredIndices[i]);
	}
}

TStatId URogueProjectilesSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueProjectilesSubsystem, STATGROUP_Tickables);
}
