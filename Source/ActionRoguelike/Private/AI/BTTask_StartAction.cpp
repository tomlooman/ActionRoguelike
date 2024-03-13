// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_StartAction.h"
#include "AIController.h"
#include "SActionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(BTTask_StartAction)


EBTNodeResult::Type UBTTask_StartAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Assumed AIController and Pawn never nullptr (BT should be disabled if so)
	APawn* MyPawn = OwnerComp.GetAIOwner()->GetPawn();
	check(MyPawn);
	
	USActionComponent* ActionComp = MyPawn->FindComponentByClass<USActionComponent>();
	// If nullptr we haven't properly implemented the enemy with an action component
	if (ensure(ActionComp))
	{
		if (ActionComp->StartActionByName(MyPawn, ActionName))
		{
			return EBTNodeResult::Succeeded;
		}
	}
	
	return EBTNodeResult::Failed;
}
