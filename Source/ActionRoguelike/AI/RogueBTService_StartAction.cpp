// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueBTService_StartAction.h"
#include "AIController.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueBTService_StartAction)

void URogueBTService_StartAction::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Assumed AIController and Pawn never nullptr (BT should be disabled if so)
	APawn* MyPawn = OwnerComp.GetAIOwner()->GetPawn();
	check(MyPawn);
	
	URogueActionComponent* ActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(MyPawn);
	check(ActionComp); // If nullptr we haven't properly implemented the enemy with an action component
	
	ActionComp->StartActionByName(MyPawn, ActionName);
}
