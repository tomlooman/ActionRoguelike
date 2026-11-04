// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor_Credits.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePickupActor_Credits)


ARoguePickupActor_Credits::ARoguePickupActor_Credits()
{
	bCanAutoPickup = true;
	CreditsAmount = 80;
}


void ARoguePickupActor_Credits::Interact_Implementation(AController* InstigatorController)
{
	APawn* MyPawn = InstigatorController->GetPawn();
	if (URogueActionComponent* ActionComp = URogueActionComponent::GetActionComponent(MyPawn))
	{
		if (CreditsAmount >= 0 && (CreditsAmount == 0 || ActionComp->ApplyAttributeChange(
			SharedGameplayTags::Attribute_Credits, CreditsAmount, this, EAttributeModifyType::AddBase)))
		{
			HideAndCooldown();
		}
	}
}

