// Fill out your copyright notice in the Description page of Project Settings.


#include "SPowerup_Action.h"
#include "SActionComponent.h"
#include "SAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SPowerup_Action)




void ASPowerup_Action::Interact_Implementation(APawn* InstigatorPawn)
{
	// Make sure an action class was set up
	if (!ensureAlways(ActionToGrant))
	{
		return;
	}

	USActionComponent* ActionComp = InstigatorPawn->FindComponentByClass<USActionComponent>();
	check(ActionComp);
	
	// Check if Player already has action class
	if (ActionComp->GetAction(ActionToGrant))
	{
		const FString DebugMsg = FString::Printf(TEXT("Action '%s' already known."), *GetNameSafe(ActionToGrant));
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
		return;
	}

	// Give new Ability
	ActionComp->AddAction(InstigatorPawn, ActionToGrant);	
	HideAndCooldownPowerup();
}
