

#include "RogueStateTreeTasks.h"

#include "StateTreeExecutionContext.h"
#include "StateTreeLinker.h"
#include "Core/RogueGameModeBase.h"
#include "StateTreePropertyRef.h"


FRogueSTTask_SelectMonster::FRogueSTTask_SelectMonster()
{
	bConsideredForScheduling = false;
}


EStateTreeRunStatus FRogueSTTask_SelectMonster::EnterState(FStateTreeExecutionContext& Context,	const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	int32 AvailableSpawnCredit = *InstanceData.AvailableCreditsRef.GetMutablePtr(Context);

	TArray<FMonsterInfoRow*> Rows;
	InstanceData.MonsterTable->GetAllRows("", Rows);

	// Get total weight
	float TotalWeight = 0;
	for (FMonsterInfoRow* Entry : Rows)
	{
		TotalWeight += Entry->Weight;
	}

	// Random number within total random
	int32 RandomWeight = FMath::RandRange(0.0f, TotalWeight);
	// @todo: make it seeded?

	//Reset
	TotalWeight = 0;

	FMonsterInfoRow* SelectedRow = nullptr;
	// Get monster based on random weight
	for (FMonsterInfoRow* Entry : Rows)
	{
		TotalWeight += Entry->Weight;

		if (RandomWeight <= TotalWeight)
		{
			SelectedRow = Entry;
			break;
		}
	}

	check(SelectedRow);
	/*if (SelectedRow == nullptr)
	{
		return EStateTreeRunStatus::Failed;
	}*/

	if (SelectedRow->SpawnCost >= AvailableSpawnCredit)
	{
		// Too expensive to spawn
		return EStateTreeRunStatus::Failed;
	}


	UClass** SelectedMonsterClass = InstanceData.SelectedMonsterRef.GetMutablePtr(Context);
	// Store it back into state tree memory
	*SelectedMonsterClass = AActor::StaticClass();

	return EStateTreeRunStatus::Running;
}
