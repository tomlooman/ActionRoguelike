
#pragma once

#include "RogueGameTypes.generated.h"

#define NAME_TargetActor "TargetActor"

#define COLLISION_INTERACTION ECC_GameTraceChannel1
#define COLLISION_PROJECTILE  ECC_GameTraceChannel2


class URogueMonsterData;
class ARogueAICharacter;
class UEnvQuery;
class UDataTable;

USTRUCT(BlueprintType)
struct FMonsterSpawnData : public FTableRowBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<URogueMonsterData> MonsterData;
	
	/* Points required by gamemode to spawn this unit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta =(ClampMin=0.0f))
	float SpawnCost = 0.0f;
	
	UPROPERTY(EditAnywhere, meta = (ClampMin=0.0f))
	float SpawnWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FRogueDirectorData
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	TObjectPtr<UEnvQuery> SpawnLocationQuery;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	TObjectPtr<UDataTable> MonsterSpawnTable;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	FRuntimeFloatCurve CreditGainCurve;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	float TickInterval = 0.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	float TimeBetweenWaves = 6.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	FString DebugDisplayName = TEXT("DirectorName");
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	FColor DebugColor = FColor::White;
	
	float CurrentCredits = 0.0f;
	
	float NextTickTime = 0.0f;
	
	FRandomStream RandomStream_MonsterSelection;
};
