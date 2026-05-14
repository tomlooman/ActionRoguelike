// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAnimInstance.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionSystemComponent.h"

void URogueAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ActionComp = GetOwningActor()->FindComponentByClass<URogueActionSystemComponent>();
}

void URogueAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	
	ActionComp->GameplayTagUpdated.AddDynamic(this, &ThisClass::OnTagUpdated);
}

void URogueAnimInstance::OnTagUpdated(FGameplayTag UpdatedTag, int32 NewCount)
{
	bool bWasAdded = NewCount > 0;
	
	if (UpdatedTag == SharedGameplayTags::StatusEffect_Sprinting)
	{
		bIsSprinting = bWasAdded;
	}
	else if (UpdatedTag == SharedGameplayTags::StatusEffect_Stunned)
	{
		bIsStunned = bWasAdded;
	}
}
