// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePlayerController.h"

#include "EnhancedInputComponent.h"
#include "RogueGameTypes.h"
#include "RogueInteractionComponent.h"

ARoguePlayerController::ARoguePlayerController()
{
	InteractionComponent = CreateDefaultSubobject<URogueInteractionComponent>(TEXT("InteractionComp"));
}

void ARoguePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	EnhancedInput->BindAction(Input_Interact, ETriggerEvent::Triggered, this, &ARoguePlayerController::StartInteract);
}

FGenericTeamId ARoguePlayerController::GetGenericTeamId() const
{
	return FGenericTeamId(TEAM_ID_PLAYERS);
}

void ARoguePlayerController::StartInteract()
{
	InteractionComponent->Interact();
}
