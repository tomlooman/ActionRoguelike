// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueGameModeBase.h"
#include "GameRules/RogueSpawnDirectors.h"
#include "RoguePrimaryGameMode.generated.h"


/* DataTable Row for spawning monsters in game mode  */
USTRUCT(BlueprintType)
struct FMonsterInfoRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	FMonsterInfoRow()
	{
		Weight = 1.0f;
		SpawnCost = 5.0f;
		KillReward = 20.0f;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId MonsterId;

	/* Relative chance to pick this monster */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight;

	/* Points required by gamemode to spawn this unit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnCost;

	/* Amount of credits awarded to killer of this unit.  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float KillReward;
};

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePrimaryGameMode : public ARogueGameModeBase
{
	GENERATED_BODY()
	
protected:
	
	/* Single game seed to generate other random streams, for consistent playback of a run */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SpawnLogic)
	float StartingSeed = 0;
	
	/* (Combat) Directors for spawning loot, monsters, events */
	UPROPERTY(EditAnywhere, Category="Spawning", meta = (TitleProperty="EditorDisplayName"))
	TArray<FRogueDirectorData> Directors;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	int32 NrMaxEnemies = 10;
	
	/*
	 * Returns TRUE if the spawn was successful, can fail if not enough credits are available
	 */
	bool TrySpawnMonster(FRogueDirectorData& DirectorData);
	
	void SpawnQueryCompleted(TSharedPtr<FEnvQueryResult> Result, FMonsterInfoRow* SelectedRow);

	void OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation);

public:
	
	virtual void StartPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;

};
