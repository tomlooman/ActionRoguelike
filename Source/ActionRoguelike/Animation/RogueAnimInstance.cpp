// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAnimInstance.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionSystemComponent.h"

void URogueAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ActionComp = GetOwningActor()->FindComponentByClass<URogueActionSystemComponent>();
}

void URogueAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (ActionComp)
	{
		bIsSprinting = ActionComp->ActiveGameplayTags.HasTag(SharedGameplayTags::StatusEffect_Sprinting);
	}
}
