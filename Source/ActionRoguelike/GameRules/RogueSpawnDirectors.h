// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQuery.h"

#include "RogueSpawnDirectors.generated.h"


class URogueMonsterData;
class UCurveFloat;

USTRUCT(BlueprintType)
struct FRogueDirectorData
{
	GENERATED_BODY()
	
	FRogueDirectorData()
	{
		CurrentCredits = StartingCredits;
	}
	
	UPROPERTY(EditAnywhere)
	float StartingCredits = 0.0f;
	
	UPROPERTY(EditAnywhere)
	float TickInterval = 1.0f;
	
	UPROPERTY(EditAnywhere)
	float TimeBetweenWaves = 5.0f;
	
	/* Default Seed, could be seeded by RNG on game start */
	UPROPERTY(EditAnywhere)
	float StartingSeed = 0;
	
	/* Curve (Nr of Credits scaling with Time) of credits gained per second. */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCurveFloat> CreditsGainCurve;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> MonsterTable;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UEnvQuery> SpawnLocationQuery;
	
	// ----- Runtime Values
	
	float CurrentCredits = 0.0f;
	
	float NextTickTime = 0.0f;

	UPROPERTY(Transient)
	TObjectPtr<URogueMonsterData> SelectedMonsterData;
		
	// Seeded by "RandomSeed" allow consistent playback of random number sequences (great for debugging and re-trying the same "playthrough" in a game session.)
	FRandomStream RandomStream_MonsterSelection;

	
#if WITH_EDITORONLY_DATA
	
	UPROPERTY(EditAnywhere)
	FString EditorDisplayName;
	
#endif
	
};


