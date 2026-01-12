// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueBTDecorator_CheckHealth.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"


void URogueBTDecorator_CheckHealth::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	// @todo: when using onhealthchanged delegate instead, you need to know:
	// - decorators share an instance, so anything must be stored inside NodeMemory instead (eg. storing the IsLowHealth bool to read inside CalculateRawConditionValue)
	// - add/remove delegate may be difficult on the class since we share this class instance with many others, the "this" param may end up accidentally removing
	// other listeners from other gameplay actors.
}

void URogueBTDecorator_CheckHealth::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

bool URogueBTDecorator_CheckHealth::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AController* Controller = Cast<AController>(OwnerComp.GetOwner());
	URogueActionComponent* ActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(Controller->GetPawn());
	
	float Health  = ActionComp->GetAttributeValue(SharedGameplayTags::Attribute_Health);
	float HealthMax = ActionComp->GetAttributeValue(SharedGameplayTags::Attribute_HealthMax);

	// Is low health?
	return (Health / HealthMax) < LowHealthFraction;
}
