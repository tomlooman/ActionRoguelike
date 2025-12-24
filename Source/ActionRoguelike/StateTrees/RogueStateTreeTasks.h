#pragma once

#include "StateTreePropertyRef.h"
#include "StateTreeTaskBase.h"
#include "RogueStateTreeTasks.generated.h"

/**
 * Configurable Vars in the State Tree in Select Monster Task
 */
USTRUCT()
struct FRogueST_SelectMonsterInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Parameter)
	UDataTable* MonsterTable = nullptr;
	
	UPROPERTY(EditAnywhere)
	TStateTreePropertyRef<int32> AvailableCreditsRef;
	
	UPROPERTY(EditAnywhere)
	TStateTreePropertyRef<UClass*> SelectedMonsterRef;
};


/**
 * Base struct to simplify exposing other Tasks to Director Schema
 */
USTRUCT(meta = (Hidden))
struct FRogueStateTreeDirectorTask : public FStateTreeTaskBase
{
	GENERATED_BODY()
};

/**
 * Select Monster to spawn from available DataTable based on available Spawn Credits
 */
USTRUCT(meta = (DisplayName = "Select Monster for Spawning"))
struct FRogueSTTask_SelectMonster : public FRogueStateTreeDirectorTask
{
	GENERATED_BODY()

	using FInstanceDataType = FRogueST_SelectMonsterInstanceData;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	FRogueSTTask_SelectMonster();
};
