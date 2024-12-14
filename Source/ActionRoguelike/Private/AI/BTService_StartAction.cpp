// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_StartAction.h"
#include "AIController.h"
#include "SActionComponent.h"

void UBTService_StartAction::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Assumed AIController and Pawn never nullptr (BT should be disabled if so)
	APawn* MyPawn = OwnerComp.GetAIOwner()->GetPawn();
	check(MyPawn);
	
	USActionComponent* ActionComp = MyPawn->FindComponentByClass<USActionComponent>();
	check(ActionComp); // If nullptr we haven't properly implemented the enemy with an action component
	
	ActionComp->StartActionByName(MyPawn, ActionName);
}
