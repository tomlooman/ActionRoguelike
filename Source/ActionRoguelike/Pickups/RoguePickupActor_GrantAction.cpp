// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor_GrantAction.h"
#include "ActionSystem/RogueActionComponent.h"
#include "ActionSystem/RogueAction.h"
#include "Core/RogueGameplayFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePickupActor_GrantAction)




void ARoguePickupActor_GrantAction::Interact_Implementation(AController* InstigatorController)
{
	// Make sure an action class was set up
	if (!ensureAlways(ActionToGrant))
	{
		return;
	}

	URogueActionComponent* ActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(InstigatorController->GetPawn());
	check(ActionComp);
	
	// Check if Player already has action class
	if (ActionComp->GetAction(ActionToGrant))
	{
		const FString DebugMsg = FString::Printf(TEXT("Action '%s' already known."), *GetNameSafe(ActionToGrant));
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, DebugMsg);
		return;
	}

	// Give new Ability
	ActionComp->AddAction(InstigatorController->GetPawn(), ActionToGrant);	
	HideAndCooldown();
}
