// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor_Credits.h"
#include "Player/RoguePlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePickupActor_Credits)


ARoguePickupActor_Credits::ARoguePickupActor_Credits()
{
	bCanAutoPickup = true;
	CreditsAmount = 80;
}


void ARoguePickupActor_Credits::Interact_Implementation(AController* InstigatorController)
{
	if (ARoguePlayerState* PS = InstigatorController->GetPlayerState<ARoguePlayerState>())
	{
		PS->AddCredits(CreditsAmount);
		HideAndCooldown();
	}
}

