// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAnimInstance.h"
#include "GameplayTagContainer.h"
#include "ActionSystem/RogueActionComponent.h"
#include "SharedGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAnimInstance)



void URogueAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	AActor* OwningActor = GetOwningActor();
	check(OwningActor);

	ActionComp = OwningActor->FindComponentByClass<URogueActionComponent>();
}



void URogueAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (ActionComp)
	{
		bIsStunned = ActionComp->ActiveGameplayTags.HasTag(SharedGameplayTags::Action_Stunned);
	}
}
