// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionEffect_Thorns.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueActionEffect_Thorns)


void URogueActionEffect_Thorns::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	// Start listening
	AttriChangedHandle = ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health)->OnAttributeChanged.AddUObject(this, &ThisClass::OnHealthChanged);
}


void URogueActionEffect_Thorns::StopAction_Implementation(AActor* Instigator)
{
	Super::StopAction_Implementation(Instigator);

	// Stop listening
	ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health)->OnAttributeChanged.Remove(AttriChangedHandle);
}


void URogueActionEffect_Thorns::OnHealthChanged(float NewValue, const FAttributeModification& AttributeModification)
{
	AActor* OwningActor = GetOwningComponent()->GetOwner();

	// Damage Only
	if (AttributeModification.Magnitude < 0.0f && OwningActor != AttributeModification.Instigator)
	{
		if (AttributeModification.ContextTags.HasTag(SharedGameplayTags::Context_Reflected))
		{
			// Skip reflected damage to avoid ping-ponging reflected damage between two actors until someone dies
			return;
		}
		
		/*
		// Round to nearest to avoid 'ugly' damage numbers and tiny reflections
		int32 ReflectedAmount = FMath::RoundToInt(AttributeModification.Magnitude * ReflectFraction);
		if (ReflectedAmount == 0)
		{
			return;
		}

		// Flip to positive, so we don't end up healing ourselves when passed into damage
		ReflectedAmount = FMath::Abs(ReflectedAmount);*/

		// @todo: maybe thorns can still base DMG on base dmg from hit rather than using player baseDmg attribute as with all normal damage
		float DmgCoefficient = 5.0f;

		FGameplayTagContainer Tags;
		Tags.AddTag(SharedGameplayTags::Context_Reflected);

		// Return damage to sender...
		URogueGameplayFunctionLibrary::ApplyDamage(OwningActor, AttributeModification.Instigator.Get(), DmgCoefficient, Tags);
	}
}
