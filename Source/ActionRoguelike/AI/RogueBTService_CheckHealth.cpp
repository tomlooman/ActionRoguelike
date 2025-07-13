// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueBTService_CheckHealth.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueBTService_CheckHealth)


void URogueBTService_CheckHealth::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	check(AIPawn);

	URogueActionComponent* ActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(AIPawn);
	check(ActionComp);

	float Health = ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health)->GetValue();
	float HealthMax = ActionComp->GetAttribute(SharedGameplayTags::Attribute_HealthMax)->GetValue();

	const bool bLowHealth = (Health / HealthMax) < LowHealthFraction;

	UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
	BlackBoardComp->SetValueAsBool(LowHealthKey.SelectedKeyName, bLowHealth);
	
}