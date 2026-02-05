// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameplayStatics.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionSystemComponent.h"
#include "ActionSystem/RogueAttributeSet.h"


bool URogueGameplayStatics::IsFullHealth(URogueActionSystemComponent* ActionComp)
{
	FRogueAttribute* Health = ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health);
	FRogueAttribute* HealthMax = ActionComp->GetAttribute(SharedGameplayTags::Attribute_HealthMax);

	return FMath::IsNearlyEqual(Health->GetValue(), HealthMax->GetValue());
}
