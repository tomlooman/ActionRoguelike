// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueBTTask_HealSelf.h"
#include "ActionSystem/RogueAttributeComponent.h"
#include "AIController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueBTTask_HealSelf)


EBTNodeResult::Type URogueBTTask_HealSelf::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	URogueAttributeComponent* AttributeComp = URogueAttributeComponent::GetAttributes(MyPawn);
	if (ensure(AttributeComp))
	{
		AttributeComp->ApplyHealthChange(MyPawn, AttributeComp->GetHealthMax());
	}

	return EBTNodeResult::Succeeded;
}

