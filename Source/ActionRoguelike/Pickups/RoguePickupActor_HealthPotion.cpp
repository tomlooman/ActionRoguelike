// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor_HealthPotion.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "Player/RoguePlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePickupActor_HealthPotion)


#define LOCTEXT_NAMESPACE "InteractableActors"



void ARoguePickupActor_HealthPotion::Interact_Implementation(AController* InstigatorController)
{
	APawn* MyPawn = InstigatorController->GetPawn();
	
	URogueActionComponent* ActionComp = URogueActionComponent::GetActionComponent(MyPawn);
	// Check if not already at max health
	if (ensure(ActionComp) && !URogueGameplayFunctionLibrary::IsFullHealth(MyPawn))
	{
		if (ARoguePlayerState* PS = InstigatorController->GetPlayerState<ARoguePlayerState>())
		{
			if (PS->TryRemoveCredits(CreditCost))
			{
				// Heal back to full
				float MaxHealing = ActionComp->GetAttribute(SharedGameplayTags::Attribute_HealthMax)->GetValue();

				if (ActionComp->ApplyAttributeChange(
					SharedGameplayTags::Attribute_Health,
					MaxHealing,
					this,
					EAttributeModifyType::AddBase))
				{
					// Only activate if healed successfully
					HideAndCooldown();
				}
			}
		}
	}
}


FText ARoguePickupActor_HealthPotion::GetInteractText_Implementation(AController* InstigatorController)
{
	// May be nullptr when dead
	APawn* MyPawn = InstigatorController->GetPawn();
	
	if (MyPawn && URogueGameplayFunctionLibrary::IsFullHealth(MyPawn))
	{
		return LOCTEXT("HealthPotion_FullHealthWarning", "Already at full health.");
	}

	return FText::Format(LOCTEXT("HealthPotion_InteractMessage", "Cost {0} Credits. Restores health to maximum."), CreditCost);
}


#undef LOCTEXT_NAMESPACE