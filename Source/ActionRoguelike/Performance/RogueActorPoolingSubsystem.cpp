// Fill out your copyright notice in the Description page of Project Settings.


#include "Performance/RogueActorPoolingSubsystem.h"

#include "ActionRoguelike.h"
#include "RogueActorPoolingInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueActorPoolingSubsystem)


static TAutoConsoleVariable CVarActorPoolingEnabled(
	TEXT("game.ActorPooling"),
	true,
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
	if (!Actor->HasAuthority())
	{
		UE_LOG(LogGame, Warning, TEXT("ReleaseToPool called from non authority. Verify the case and skip calling this function on clients."));
		return false;
	}
	
	if (IsPoolingEnabled())
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
	if (IsPoolingEnabled())
	{
		URogueActorPoolingSubsystem* PoolingSystem = WorldContextObject->GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
		return PoolingSystem->AcquireFromPool_Internal(ActorClass, SpawnTransform, SpawnParams);
	}
	
	SCOPED_NAMED_EVENT(SpawnActorNoPool, FColor::Red);
	// Fallback to standard spawning when not enabled
	return WorldContextObject->GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);
}


void URogueActorPoolingSubsystem::PrimeActorPool(TSubclassOf<AActor> ActorClass, int32 Amount)
{
	UE_LOGFMT(LogGame, Log, "Priming Pool for {actorclass} ({amount})", GetNameSafe(ActorClass), Amount);
	SCOPED_NAMED_EVENT(PrimeActorPool, FColor::Blue);

	// Disallow priming on clients, they will receive their actors through replication to keep it simple.
	check(!GetWorld()->IsNetMode(NM_Client));
	
	// Prime a set number of pooled actors, this reduces memory fragmentation and any potential initial hitches during gameplay
	for (int i = 0; i < Amount; ++i)
	{
		// Place far below, but still within default "killz" bounds
		FTransform FarAwayTM;
		FarAwayTM.SetLocation(FVector(0.0f, 0.0f, -(UE_OLD_HALF_WORLD_MAX - 10)));

		// Deferred spawn, avoid any collision during construction/beginplay
		AActor* DeferredActor = GetWorld()->SpawnActorDeferred<AActor>(ActorClass, FTransform::Identity,
			nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		// Avoid any initial overlaps on spawn
		DeferredActor->SetActorEnableCollision(false);
		// Skip any initial replications
		// @todo: disabled for now
		//DeferredActor->SetNetDormancy(ENetDormancy::DORM_DormantAll);

		DeferredActor->FinishSpawning(FTransform::Identity, true, nullptr, ESpawnActorScaleMethod::MultiplyWithRoot);

		ReleaseToPool_Internal(DeferredActor);

		// Make sure it did not get deleted
		check(IsValid(DeferredActor));
	}
}


bool URogueActorPoolingSubsystem::ReleaseToPool_Internal(AActor* ActorToFree)
{
	SCOPED_NAMED_EVENT(ReleaseActorToPool, FColor::White);
	check(IsValid(ActorToFree));
	
	UE_LOGFMT(LogGame, Log, "Releasing Actor for {actorclass} to pool", GetNameSafe(ActorToFree->GetClass()));
	
	// Avoid calling this on client, only supporting SERVER side to acquire the actors for game AI based actors/projectiles
    // Network Replication will handle waking up and updating the Actor instead
    check(!GetWorld()->IsNetMode(NM_Client));
	
	ParkActor(ActorToFree);
		
	// Only relevant to set on the host
	// @todo: this might conflict with us needing to replicate the bool to the client to make them park the actor
	// simple solution is to delay this dormancy
	//ActorToFree->SetNetDormancy(ENetDormancy::DORM_DormantAll);

	// Place in the pool for later use
	FActorPool* ActorPool = &AvailableActorPool.FindOrAdd(ActorToFree->GetClass());
	ActorPool->FreeActors.Add(ActorToFree);

	// Debug bookkeeping
	ActorPool->InUseActors.RemoveSingleSwap(ActorToFree);

	return true;
}

void URogueActorPoolingSubsystem::ActivateActor(AActor* InActor)
{
	// These are assumed not used by game code
	InActor->SetActorEnableCollision(true);
	InActor->SetActorHiddenInGame(false);
	
	IRogueActorPoolingInterface::Execute_PoolBeginPlay(InActor);

	InActor->DispatchBeginPlay();

}

void URogueActorPoolingSubsystem::ParkActor(AActor* InActor)
{
	// These actors are assumed not used by game code
	InActor->SetActorEnableCollision(false);
	InActor->SetActorHiddenInGame(true);

	// False for those we just spawned for priming
	if (InActor->HasActorBegunPlay())
	{
		InActor->RouteEndPlay(EEndPlayReason::Destroyed);
		IRogueActorPoolingInterface::Execute_PoolEndPlay(InActor);
	}
}


void URogueActorPoolingSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsPoolingEnabled())
	{
		UWorld* World = GetWorld();

		// todo: this list isn't matched on the clients, so they don't see the same debug drawing

		for (auto Pool : AvailableActorPool)
		{
			for (AActor* UsedActor : Pool.Value.InUseActors)
			{
				// draw in-world debug info
				DrawDebugBox(World, UsedActor->GetActorLocation(), FVector(20.0f), FColor::Green, false, 0.0f, SDPG_Foreground);
			}
		}
	}
}



AActor* URogueActorPoolingSubsystem::AcquireFromPool_Internal(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams)
{
	SCOPED_NAMED_EVENT(AcquireActorFromPool, FColor::White);
	
	// Avoid calling this on client, only supporting SERVER side to acquire the actors for game AI based actors/projectiles
	// Network Replication will handle waking up and updating the Actor instead
	check(!GetWorld()->IsNetMode(NM_Client));
	
	AActor* AcquiredActor = nullptr;

	FActorPool* ActorPool = &AvailableActorPool.FindOrAdd(ActorClass);
	// Grab first available
	if (ActorPool->FreeActors.IsValidIndex(0))
	{
		UE_LOGFMT(LogGame, Log, "Acquired Actor for {actorclass} from pool", GetNameSafe(ActorClass));
		
		AcquiredActor = ActorPool->FreeActors[0];

		// Remove from pool
		ActorPool->FreeActors.RemoveAtSwap(0, 1, EAllowShrinking::No);
		// Debug bookkeeping
		ActorPool->InUseActors.Add(AcquiredActor);
	}

	// Failed to find actor
	if (AcquiredActor == nullptr)
	{
		// Marked warning, ideally we rely on primed actors instead
		UE_LOGFMT(LogGame, Warning, "Actor Pool empty, spawning new Actor for {actorclass}", GetNameSafe(ActorClass));
		
		// Spawn fresh instance that can eventually be release to the pool
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);

		ActorPool->InUseActors.Add(NewActor);

		return NewActor;
	}

	// Wake up before making any changes to replicated properties
	// @todo: disabled for now
	//AcquiredActor->SetNetDormancy(ENetDormancy::DORM_Awake);

	ActivateActor(AcquiredActor);

	// These will be replicated back to the clients	
	AcquiredActor->SetActorTransform(SpawnTransform);
	AcquiredActor->SetInstigator(SpawnParams.Instigator);
	AcquiredActor->SetOwner(SpawnParams.Owner);

	return AcquiredActor;
}


TStatId URogueActorPoolingSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(RogueActorPoolingSubsystem, STATGROUP_Tickables);
}


bool URogueActorPoolingSubsystem::IsPoolingEnabled()
{
	return CVarActorPoolingEnabled.GetValueOnAnyThread();
}
