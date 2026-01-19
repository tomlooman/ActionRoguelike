// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueMonsterCorpseSubsystem.h"



void URogueMonsterCorpseSubsystem::AddCorpse(AActor* InActor)
{
	Corpses.Enqueue(FMonsterCorpseInfo(InActor, GetWorld()->TimeSeconds));

	CurrentCorpseCount++;
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
		// Alternatively we could hide the corpse and apply pooling on "corpses", simple actors containing only a skeletalmesh for ragdolling 
		NextCorpse->Actor->Destroy();

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