// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueBTTask_HealSelf.h"
#include "AIController.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueBTTask_HealSelf)


EBTNodeResult::Type URogueBTTask_HealSelf::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	APawn* MyPawn = OwnerComp.GetAIOwner()->GetPawn();

	URogueActionComponent* ActionComp = URogueActionComponent::GetActionComponent(MyPawn);
	check (ActionComp);
	
	const FRogueAttribute* HealthMaxAttribute = ActionComp->GetAttribute(SharedGameplayTags::Attribute_HealthMax);

	ActionComp->ApplyAttributeChange(SharedGameplayTags::Attribute_Health, HealthMaxAttribute->GetValue(),
		// @todo: support CLAMPING in the attribute system, now it overheals the BOT beyond original max HP
		MyPawn, EAttributeModifyType::AddModifier);

	return EBTNodeResult::Succeeded;
}

