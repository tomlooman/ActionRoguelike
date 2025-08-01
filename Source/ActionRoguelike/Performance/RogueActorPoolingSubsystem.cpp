﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "Performance/RogueActorPoolingSubsystem.h"

#include "ActionRoguelike.h"
#include "RogueActorPoolingInterface.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueActorPoolingSubsystem)


static TAutoConsoleVariable CVarActorPoolingEnabled(
	TEXT("game.ActorPooling"),
	false, // Disabled by default in sample project. Has some issues to work out (such as properly resetting all VFX like the black hole projectile)
	TEXT("Enable actor pooling for selected objects."),
	ECVF_Default);


AActor* URogueActorPoolingSubsystem::SpawnActorPooled(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod SpawnHandling)
{
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = SpawnHandling;
	
	return AcquireFromPool(WorldContextObject, ActorClass, SpawnTransform, Params);
}

bool URogueActorPoolingSubsystem::ReleaseToPool(AActor* Actor)
{
	if (IsPoolingEnabled(Actor))
	{
		URogueActorPoolingSubsystem* PoolingSubsystem = Actor->GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
		return PoolingSubsystem->ReleaseToPool_Internal(Actor);
	}

	SCOPED_NAMED_EVENT(DestroyActorNoPool, FColor::Red);
	Actor->Destroy();
	return false;
}


AActor* URogueActorPoolingSubsystem::AcquireFromPool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams)
{
	if (IsPoolingEnabled(WorldContextObject))
	{
		URogueActorPoolingSubsystem* PoolingSystem = WorldContextObject->GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
		return PoolingSystem->AcquireFromPool_Internal(ActorClass, SpawnTransform, SpawnParams);
	}
	
	SCOPED_NAMED_EVENT(SpawnActorNoPool, FColor::Red);
	// Fallback to standard spawning when not enabled
	return WorldContextObject->GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);
}


bool URogueActorPoolingSubsystem::IsPoolingEnabled(const UObject* WorldContextObject)
{
	return CVarActorPoolingEnabled.GetValueOnAnyThread() && WorldContextObject->GetWorld()->IsNetMode(NM_Standalone);
}

void URogueActorPoolingSubsystem::PrimeActorPool(TSubclassOf<AActor> ActorClass, int32 Amount)
{
	UE_LOGFMT(LogGame, Log, "Priming Pool for {actorclass} ({amount})", GetNameSafe(ActorClass), Amount);
	SCOPED_NAMED_EVENT(PrimeActorPool, FColor::Blue);
	
	// Prime a set number of pooled actors, this reduces memory fragmentation and any potential initial hitches during gameplay
	for (int i = 0; i < Amount; ++i)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// @fixme: this can trigger an overlap when spawned, for example in empty level it may overlap SpectatorPawn in world zero
		// ideally we can prevent the projectile from fully activating to avoid this overlap
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, FTransform::Identity, Params);

		ReleaseToPool(NewActor);
	}
}


bool URogueActorPoolingSubsystem::ReleaseToPool_Internal(AActor* Actor)
{
	SCOPED_NAMED_EVENT(ReleaseActorToPool, FColor::White);
	check(IsValid(Actor));

	// These are assumed not used by game code
	Actor->SetActorEnableCollision(false);
	Actor->SetActorHiddenInGame(true);

	Actor->RouteEndPlay(EEndPlayReason::Destroyed);

	IRogueActorPoolingInterface::Execute_PoolEndPlay(Actor);

	// Place in the pool for later use
	FActorPool* ActorPool = &AvailableActorPool.FindOrAdd(Actor->GetClass());
	ActorPool->FreeActors.Add(Actor);

	return true;
}


AActor* URogueActorPoolingSubsystem::AcquireFromPool_Internal(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams)
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
		// @todo: keep in pool but mark as in-use
		ActorPool->FreeActors.RemoveAtSwap(0, 1, EAllowShrinking::No);
	}

	// Failed to find actor
	if (AcquiredActor == nullptr)
	{
		UE_LOGFMT(LogGame, Log, "Actor Pool empty, spawning new Actor for {actorclass}", GetNameSafe(ActorClass));
		
		// Spawn fresh instance that can eventually be release to the pool
		return GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);
	}
	
	AcquiredActor->SetActorTransform(SpawnTransform);
	AcquiredActor->SetInstigator(SpawnParams.Instigator);
	AcquiredActor->SetOwner(SpawnParams.Owner);

	// These are assumed not used by game code
	AcquiredActor->SetActorEnableCollision(true);
	AcquiredActor->SetActorHiddenInGame(false);

	AcquiredActor->DispatchBeginPlay();
	
	IRogueActorPoolingInterface::Execute_PoolBeginPlay(AcquiredActor);

	return AcquiredActor;
}
