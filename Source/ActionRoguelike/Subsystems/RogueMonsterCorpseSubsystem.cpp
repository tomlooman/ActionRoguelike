// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueMonsterCorpseSubsystem.h"

#include "Performance/RogueActorPoolingSubsystem.h"
#include "World/RogueMonsterCorpse.h"


ARogueMonsterCorpse* URogueMonsterCorpseSubsystem::FetchCorpse(AActor* InActor, URogueMonsterData* MonsterData)
{
	CurrentCorpseCount++;
	
	FTransform SpawnTM = FTransform(InActor->GetActorRotation(), InActor->GetActorLocation());

	URogueActorPoolingSubsystem* Pooler = GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();

	ARogueMonsterCorpse* PooledCorpse = Pooler->AcquireFromPool<ARogueMonsterCorpse>(ARogueMonsterCorpse::StaticClass(), SpawnTM);
	PooledCorpse->SetCorpseProperties(InActor->FindComponentByClass<USkeletalMeshComponent>(), MonsterData);

	// Track to clean up after some time or limit reached
	Corpses.Enqueue(FMonsterCorpseInfo(PooledCorpse, GetWorld()->TimeSeconds));

	return PooledCorpse;
}


void URogueMonsterCorpseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	URogueActorPoolingSubsystem* Pooler = GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
	Pooler->PrimeActorPool(ARogueMonsterCorpse::StaticClass(), MaxCorpses);
}


void URogueMonsterCorpseSubsystem::CleanupNextAvailableCorpse()
{
	if (Corpses.IsEmpty())
	{
		return;
	}
	
	FMonsterCorpseInfo* NextCorpse = Corpses.Peek();

	// Check if we reached the minimum age yet
	const float ExpiresAt = NextCorpse->TimeAdded + MinimumAge;
	if (GetWorld()->TimeSeconds < ExpiresAt)
	{
		return;
	}

	// If off-screen we can safely hide/delete - include fallback when above hard-limit, we always delete first even if potentially on-screen.
	if (CurrentCorpseCount > MaxCorpses || !NextCorpse->Actor->WasRecentlyRendered())
	{
		URogueActorPoolingSubsystem* Pooler = GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
		Pooler->ReleaseToPool(NextCorpse->Actor.Get());

		Corpses.Dequeue();
		CurrentCorpseCount--;
	}
}


void URogueMonsterCorpseSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Cleanup at most one per frame by peeking the queue
	CleanupNextAvailableCorpse();
}