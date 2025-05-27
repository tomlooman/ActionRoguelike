// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueBTTask_StartAction.h"
#include "AIController.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueBTTask_StartAction)


EBTNodeResult::Type URogueBTTask_StartAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Assumed AIController and Pawn never nullptr (BT should be disabled if so)
	APawn* MyPawn = OwnerComp.GetAIOwner()->GetPawn();
	check(MyPawn);
	
	URogueActionComponent* ActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(MyPawn);
	check(ActionComp); // If nullptr we haven't properly implemented the enemy with an action component
	
	if (ActionComp->StartActionByName(MyPawn, ActionName))
	{
		return EBTNodeResult::Succeeded;
	}
	
	return EBTNodeResult::Failed;
}
