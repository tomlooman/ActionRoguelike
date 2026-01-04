// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectilesSubsystem.h"

#include "ActionRoguelike.h"
#include "GenericTeamAgentInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraDataChannel.h"
#include "NiagaraDataChannelAccessor.h"
#include "NiagaraFunctionLibrary.h"
#include "RogueProjectileData.h"
#include "Core/RogueGameState.h"
#include "ProfilingDebugging/CountersTrace.h"

TRACE_DECLARE_INT_COUNTER(LightweightProjectilesCount, TEXT("Game/DataOnlyProjectileCount"));


int32 URogueProjectilesSubsystem::CreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor)
{
	UWorld* World = GetWorld();
		
	uint32 InstanceID = GetUniqueProjID(InPosition, World->TimeSeconds);
	
	if (HasAuthority())
	{
		InternalCreateProjectile(InPosition, InDirection, ProjectileConfig, InstigatorActor, InstanceID);
	}
	else // Client
	{
		// Push to server, locally we also create the pojectile immediately to avoid any latency
		ARogueGameState* GS = World->GetGameState<ARogueGameState>();
		GS->ServerCreateProjectile(InPosition, InDirection, ProjectileConfig, InstigatorActor, InstanceID);
		
		InternalCreateProjectile(InPosition, InDirection, ProjectileConfig, InstigatorActor, InstanceID);
	}

	return InstanceID;
}

// @todo: rename as this is used 'outside' this class too
void URogueProjectilesSubsystem::InternalCreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor, uint32 NewID)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CreateProjectile);

	check(ProjectileConfig);

	// for clients, we first locate an existing projectile inst as we might have spawned it ourselves
	FProjectileInstance* ExistingInst = ProjectileInstances.FindByPredicate([NewID](FProjectileInstance& Item){ return NewID == Item.ID;  });
	if (ExistingInst)
	{
		// Everything is setup if we already created the projectile earlier als client.
		// @todo-test: what happens if we hit immediately after spawning, even before the server received and send back the info...
		return;	
	}
		
	UWorld* World = GetWorld();
	// Now assign the FX Comp to the ProjectileConfig already in the local array
	FProjectileArray& ProjectileConfigs = World->GetGameState<ARogueGameState>()->ProjectileData;
	
	// Active data that represents the moving projectile
	FProjectileInstance ProjInfo = FProjectileInstance(InPosition, InDirection * ProjectileConfig->InitialSpeed, NewID);
	ProjectileInstances.Add(ProjInfo);

	UNiagaraComponent* EffectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		World, ProjectileConfig->ProjectileEffect, InPosition,
		InDirection.Rotation(), FVector(1), true, true, ENCPoolMethod::AutoRelease);

	// The component stays in world space, the emitters themselves move along the axis of the projectile to match
	// the collision query locations
	const FName UserParamName = "ProjectileVelocity";
	FVector LocalVelocity = FVector::ForwardVector * ProjectileConfig->InitialSpeed;
	EffectComp->SetVariablePosition(UserParamName, LocalVelocity);

	// May exist on the client that spawned this through "prediction"
	FProjectileItem* Data = ProjectileConfigs.Items.FindByPredicate(
	[NewID](FProjectileItem& Item){ return NewID == Item.ID;  });

	// Expected on server or when a client spawns his own projectile before calling the server (RPC)
	if (Data)
	{
		Data->TracerEffectComp = EffectComp;
	}
	else 
	{
		float ExpirationGameTime = World->TimeSeconds + ProjectileConfig->Lifespan;
		// Full data of the projectile instance, used for bookkeeping and replication, not constantly updated
		FProjectileItem Info = FProjectileItem(InPosition, InDirection, ProjectileConfig, InstigatorActor, NewID, ExpirationGameTime);
		
		Info.TracerEffectComp = EffectComp;

		// @todo: unclear if this is OK to add to array on clients, as we may desync the array between server/client
		// if not, I may need to keep a local array as temp lookup until the server updates his array.
		ProjectileConfigs.Items.Add(Info);
		ProjectileConfigs.MarkItemDirty(Info);

	}
}


void URogueProjectilesSubsystem::RemoveProjectileID(uint32 IdToRemove)
{
	UWorld* World = GetWorld();

	FProjectileArray& DataArray = World->GetGameState<ARogueGameState>()->ProjectileData;

	// @todo: faster lookup available like a local TMap cache
	FProjectileItem& ProjConfig = *DataArray.Items.FindByPredicate([IdToRemove](FProjectileItem& Item){ return IdToRemove == Item.ID;  });
	
	if (ProjConfig.TracerEffectComp)
	{
		ProjConfig.TracerEffectComp->Deactivate();
		//FXComp->ReleaseToPool();
		ProjConfig.TracerEffectComp = nullptr;
	}

	if (ProjConfig.Hit.GetActor() && !ProjConfig.bHasPlayedImpact)
	{
		ProjConfig.bHasPlayedImpact = true;

		// Projectile direction, good for orienting the VFX, decal will use surface normal instead
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

	FProjectileArray& ProjectileConfigs = World->GetGameState<ARogueGameState>()->ProjectileData;

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

			{
				// Debug Only
				//FProjectileConfig& ProjConfig = *ProjectileConfigs.Items.FindByPredicate([Proj](const FProjectileConfig& OtherProj){ return Proj.ID == OtherProj.ID;  });

				// We are somehow not seeing consistent tracer lifetimes
				//check(ProjConfig.TracerEffectComp && ProjConfig.TracerEffectComp->IsActive());

				//DrawDebugPoint(World, Proj.Position, 10.0f, FColor::Green, false, 2.0f);
			}
			
			TArray<FHitResult> HitResults;		
			// True only blocking hit
			if (World->SweepMultiByChannel(HitResults, Proj.Position, NextPosition, FQuat::Identity, CollisionChannel, Shape))
			{
				// As far as we can move till hit
				Proj.Position = HitResults.Last().Location;
				
				FProjectileItem& ProjConfig = *ProjectileConfigs.Items.FindByPredicate([Proj](const FProjectileItem& OtherProj){ return Proj.ID == OtherProj.ID;  });
		
				//DrawDebugSphere(World, Proj.Position, Shape.GetSphereRadius(), 20, FColor::Green, false, 2.0f);
		
				// (Blocking) Hit!
				ProjConfig.Hit = HitResults[0];
				
				ProjectileHitIDs.Add(Proj.ID);


				// @todo: is auth check required?
				//if (HasAuthority())
				{
					// Give a bit of time before deletion for clients to rep the Hit
					ProjConfig.ExpirationGameTime = (World->TimeSeconds + 1.0);
					// Push hit to clients
					ProjectileConfigs.MarkItemDirty(ProjConfig);
				}
				continue;
			}
			// Any overlaps?
			for (int32 HitIndex = 0; HitIndex < HitResults.Num(); HitIndex++)
			{
				FHitResult& Hit = HitResults[HitIndex];
				AActor* HitActor = Hit.GetActor();

				FProjectileItem& ProjConfig = *ProjectileConfigs.Items.FindByPredicate([Proj](const FProjectileItem& OtherProj){ return Proj.ID == OtherProj.ID;  });
			
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
					
					// @todo: is auth check required?
					//if (HasAuthority())
					{
						// Give a bit of time before deletion for clients to rep the Hit
						ProjConfig.ExpirationGameTime = (World->TimeSeconds + 1.0);
						// Push hit to clients
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
			
			FProjectileItem& ProjConfig = *ProjectileConfigs.Items.FindByPredicate([HitID](const FProjectileItem& OtherProj){ return HitID == OtherProj.ID;  });

			// Apply Damage (server-only, misc. stuff like impulses still runs on clients too)
			// @todo-fixme: disabled damage to make things much easier to test
			/*FGameplayTagContainer ContextTags;
			URogueGameplayFunctionLibrary::ApplyDirectionalDamage(ProjConfig.InstigatorActor, ProjConfig.Hit.GetActor(),
				ProjConfig.ConfigDataAsset->DamageCoefficient, ProjConfig.Hit, ContextTags);*/
			// @todo: should clients be allowed to tell server we hit something IF that client spawned it? For co-op this could help with gamefeel as projectiles you "see" hitting
			// might not register as hits on server if the latency is big enough and objects are moving.
			
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
			FProjectileItem& Item = ProjectileConfigs.Items[Index];
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


void URogueProjectilesSubsystem::SpawnImpactFX(const UWorld* World, const FProjectileItem& ProjConfig, FVector ImpactPosition, FRotator ImpactRotation)
{
	// Impact Explosion
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, ProjConfig.ConfigDataAsset->ImpactEffect, ImpactPosition, ImpactRotation,
		FVector(1), true, true, ENCPoolMethod::AutoRelease, true);
	
	// Skip on movables and non-receivers
	UPrimitiveComponent* HitComp = ProjConfig.Hit.GetComponent();

#if !UE_BUILD_SHIPPING
	if (HitComp == nullptr)
	{
		UE_LOG(LogGame, Warning, TEXT("Hit something without component, reconsider the collision profiles to skip this Actor (%s)."), *GetNameSafe(ProjConfig.Hit.GetActor()));
	}
#endif

	TRACE_CPUPROFILER_EVENT_SCOPE(InitImpactDecalParticle);
	
	if (HitComp && HitComp->bReceivesDecals && HitComp->GetMobility() != EComponentMobility::Type::Movable)
	{
		// Helps find the correct island to inject this particle into
		FNiagaraDataChannelSearchParameters Params = FNiagaraDataChannelSearchParameters(ImpactPosition);

		// DECAL, using the Data Channels rather than relying on individual particle systems
		UNiagaraDataChannelWriter* Writer = UNiagaraDataChannelLibrary::WriteToNiagaraDataChannel(World, ProjConfig.ConfigDataAsset->ImpactDecal_DataChannel,
			Params, 1, false, false, false, "ImpactDecals");

		Writer->WriteVector("ImpactLocation", 0, ImpactPosition);
		Writer->WriteVector("ImpactNormal", 0, ProjConfig.Hit.ImpactNormal);
	}
}

/* Get unique ID so that client and servers can both generate an ID that will be unique when using this in replication
 * Clients may spawn a projectile locally and send it to the server along with the ID. 
 */
uint32 URogueProjectilesSubsystem::GetUniqueProjID(FVector InPos, float InGameTime)
{
	uint32 HashA = GetTypeHash(InPos);
	uint32 HashC = GetTypeHash(InGameTime);

	return HashCombine(HashA, HashC);
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
