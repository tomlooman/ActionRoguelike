// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor_HealthPotion.h"
#include "ActionSystem/RogueAttributeComponent.h"
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

	URogueAttributeComponent* AttributeComp = URogueAttributeComponent::GetAttributes(InstigatorPawn);
	// Check if not already at max health
	if (ensure(AttributeComp) && !AttributeComp->IsFullHealth())
	{
		if (ARoguePlayerState* PS = InstigatorPawn->GetPlayerState<ARoguePlayerState>())
		{
			if (PS->RemoveCredits(CreditCost) && AttributeComp->ApplyHealthChange(this, AttributeComp->GetHealthMax()))
			{
				// Only activate if healed successfully
				HideAndCooldown();
			}
		}
	}
}


FText ARoguePickupActor_HealthPotion::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	URogueAttributeComponent* AttributeComp = URogueAttributeComponent::GetAttributes(InstigatorPawn);
	if (AttributeComp && AttributeComp->IsFullHealth())
	{
		return LOCTEXT("HealthPotion_FullHealthWarning", "Already at full health.");
	}

	return FText::Format(LOCTEXT("HealthPotion_InteractMessage", "Cost {0} Credits. Restores health to maximum."), CreditCost);
}


#undef LOCTEXT_NAMESPACE