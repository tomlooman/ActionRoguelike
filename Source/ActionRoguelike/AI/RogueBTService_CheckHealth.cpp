// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueBTService_CheckHealth.h"
#include "ActionSystem/RogueAttributeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueBTService_CheckHealth)


URogueBTService_CheckHealth::URogueBTService_CheckHealth()
{
	LowHealthFraction = 0.3f;
}


void URogueBTService_CheckHealth::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();

	URogueAttributeComponent* AttributeComp = URogueAttributeComponent::GetAttributes(AIPawn);
	if (ensure(AttributeComp))
	{
		const bool bLowHealth = (AttributeComp->GetHealth() / AttributeComp->GetHealthMax()) < LowHealthFraction;

		UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
		BlackBoardComp->SetValueAsBool(LowHealthKey.SelectedKeyName, bLowHealth);
	}
}