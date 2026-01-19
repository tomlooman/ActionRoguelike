// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Containers/SpscQueue.h"
#include "RogueMonsterCorpseSubsystem.generated.h"


USTRUCT()
struct FMonsterCorpseInfo
{
	GENERATED_BODY()

	FMonsterCorpseInfo() {}

	FMonsterCorpseInfo(AActor* InActor, float TimeOfDeath)
		: Actor(InActor), TimeAdded(TimeOfDeath)
	{}

	float TimeAdded = 0.0f;

	TWeakObjectPtr<AActor> Actor = nullptr;
};


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueMonsterCorpseSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(URogueMonsterCorpseSubsystem, STATGROUP_Tickables);
	}

public:
	
	void AddCorpse(AActor* InActor);

protected:

	/*
	 * Look at the next in line and see if they are offscreen, ready to be removed
	 */
	void CleanupNextAvailableCorpse();
	
	/* Corpes stay around for at least this long */
	float MinimumAge = 10;

	int32 MaxCorpses = 5;

	int32 CurrentCorpseCount = 0;
	
	//UPROPERTY()
	TSpscQueue<FMonsterCorpseInfo> Corpses;

};
