// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/SActorPoolingSubsystem.h"

#include "ActionRoguelike.h"
#include "Logging/StructuredLog.h"


// LifeSpanExpired needs to skip Destroy() on ACTOR_HAS_LABELS
// Maybe skip world bounds and fell out of world



bool USActorPoolingSubsystem::ReleaseToPool(AActor* Actor)
{
	SCOPED_NAMED_EVENT(ReleaseActorToPool, FColor::White);
	
	// properly de-init the actor

	check(IsValid(Actor));

	// These are assumed not used by game code
	Actor->SetActorEnableCollision(false);
	Actor->SetActorHiddenInGame(true);


	// clear any timers on Actor

	// clear any "latent" actions

	// detach any actors	
	
	// Routes EndPlay
	Actor->Destroyed();

	Actor->UnregisterAllComponents();

	// de-register any tick functions

	// Place in the pool for later use
	FActorPool* ActorPool = &AvailableActorPool.FindOrAdd(Actor->GetClass());
	ActorPool->FreeActors.Add(Actor);

	return true;
}

AActor* USActorPoolingSubsystem::AquireFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams)
{
	SCOPED_NAMED_EVENT(AcquireActorFromPool, FColor::White);
	
	AActor* AcquiredActor = nullptr;

	FActorPool* ActorPool = &AvailableActorPool.FindOrAdd(ActorClass);
	// Grab first available
	if (ActorPool->FreeActors.IsValidIndex(0))
	{
		UE_LOGFMT(LogGame, Log, "Acquired Actor for {actorclass} from pool", GetNameSafe(ActorClass));
		
		AcquiredActor = ActorPool->FreeActors[0];

		// Remove from pool
		ActorPool->FreeActors.RemoveAt(0, 1, false);
	}

	// Failed to find actor
	if (AcquiredActor == nullptr)
	{
		UE_LOGFMT(LogGame, Log, "Actor Pool empty, spawning new Actor for {actorclass}", GetNameSafe(ActorClass));
		
		// Spawn fresh instance that can eventually be release to the pool
		return GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);
	}

	// @todo: maybe I should serialize the default state to overwrite important variables
	
	// These are assumed not used by game code
	AcquiredActor->SetActorEnableCollision(true);
	AcquiredActor->SetActorHiddenInGame(false);

	// Re-init so we may begin with a clean slate

	AcquiredActor->SetActorTransform(SpawnTransform);
	AcquiredActor->SetInstigator(SpawnParams.Instigator);
	AcquiredActor->SetOwner(SpawnParams.Owner);

	// re-register components

	AcquiredActor->RegisterAllComponents();

	// start the tick functions

	AcquiredActor->DispatchBeginPlay();

	return AcquiredActor;
}


AActor* USActorPoolingSubsystem::GetPooledActor(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams)
{
	USActorPoolingSubsystem* PoolingSystem = WorldContextObject->GetWorld()->GetSubsystem<USActorPoolingSubsystem>();
	return PoolingSystem->AquireFromPool(ActorClass, SpawnTransform, SpawnParams);
}
