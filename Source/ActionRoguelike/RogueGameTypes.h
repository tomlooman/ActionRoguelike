
#pragma once

#include "RogueGameTypes.generated.h"

#define NAME_TargetActor "TargetActor"

#define COLLISION_INTERACTION ECC_GameTraceChannel1
#define COLLISION_PROJECTILE  ECC_GameTraceChannel2


class ARogueAICharacter;
class UEnvQuery;
class UDataTable;

USTRUCT(BlueprintType)
struct FMonsterSpawnData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<ARogueAICharacter> MonsterClass;
	
	/* Points required by gamemode to spawn this unit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnCost = 0.0f;
	
	UPROPERTY(EditAnywhere)
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
	
	float CurrentCredits = 0.0f;
	
	float NextTickTime = 0.0f;
	
	FRandomStream RandomStream_MonsterSelection;
};
