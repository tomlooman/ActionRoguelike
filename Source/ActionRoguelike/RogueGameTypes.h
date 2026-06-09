
#pragma once

#include "RogueGameTypes.generated.h"

#define NAME_TargetActor "TargetActor"

#define COLLISION_INTERACTION ECC_GameTraceChannel1
#define COLLISION_PROJECTILE  ECC_GameTraceChannel2


class ARogueAICharacter;

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
};