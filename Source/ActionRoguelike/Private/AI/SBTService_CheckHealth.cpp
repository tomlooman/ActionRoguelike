// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SBTService_CheckHealth.h"
#include "SAttributeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SBTService_CheckHealth)


USBTService_CheckHealth::USBTService_CheckHealth()
{
	LowHealthFraction = 0.3f;
}


void USBTService_CheckHealth::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();

	USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(AIPawn);
	if (ensure(AttributeComp))
	{
		const bool bLowHealth = (AttributeComp->GetHealth() / AttributeComp->GetHealthMax()) < LowHealthFraction;

		UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
		BlackBoardComp->SetValueAsBool(LowHealthKey.SelectedKeyName, bLowHealth);
	}
}