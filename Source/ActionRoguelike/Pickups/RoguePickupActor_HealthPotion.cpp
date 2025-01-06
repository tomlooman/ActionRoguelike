// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor_HealthPotion.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "Player/RoguePlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePickupActor_HealthPotion)


#define LOCTEXT_NAMESPACE "InteractableActors"


ARoguePickupActor_HealthPotion::ARoguePickupActor_HealthPotion()
{
	CreditCost = 50;
}


void ARoguePickupActor_HealthPotion::Interact_Implementation(APawn* InstigatorPawn)
{
	if (!ensure(InstigatorPawn))
	{
		return;
	}

	URogueActionComponent* ActionComp = URogueActionComponent::GetActionComponent(InstigatorPawn);
	// Check if not already at max health
	if (ensure(ActionComp) && !URogueGameplayFunctionLibrary::IsFullHealth(InstigatorPawn))
	{
		if (ARoguePlayerState* PS = InstigatorPawn->GetPlayerState<ARoguePlayerState>())
		{
			
			if (PS->TryRemoveCredits(CreditCost))
			{
				if (ActionComp->ApplyAttributeChange(
					SharedGameplayTags::Attribute_Health,
					ActionComp->GetAttribute(SharedGameplayTags::Attribute_HealthMax)->GetValue(),
					this,
					EAttributeModifyType::AddModifier))
				{
					// Only activate if healed successfully
					HideAndCooldown();
				}
			}
		}
	}
}


FText ARoguePickupActor_HealthPotion::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	if (URogueGameplayFunctionLibrary::IsFullHealth(InstigatorPawn))
	{
		return LOCTEXT("HealthPotion_FullHealthWarning", "Already at full health.");
	}

	return FText::Format(LOCTEXT("HealthPotion_InteractMessage", "Cost {0} Credits. Restores health to maximum."), CreditCost);
}


#undef LOCTEXT_NAMESPACE