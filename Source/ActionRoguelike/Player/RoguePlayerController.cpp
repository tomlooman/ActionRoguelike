// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePlayerController.h"

#include "EnhancedInputComponent.h"
#include "RogueInteractionComponent.h"
#include "Development/RogueCheatManager.h"
#include "UI/RogueHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePlayerController)



ARoguePlayerController::ARoguePlayerController()
{
	CheatClass = URogueCheatManager::StaticClass();

	// For multiplayer, its better to have this here so it only runs on the owning client (and the server, which can disable the tick)
	InteractionComp = CreateDefaultSubobject<URogueInteractionComponent>(TEXT("InteractionComp"));
}


void ARoguePlayerController::PrimaryInteract()
{
	InteractionComp->PrimaryInteract();
}


void ARoguePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EnhInputComp = CastChecked<UEnhancedInputComponent>(InputComponent);
	EnhInputComp->BindAction(Input_Interact, ETriggerEvent::Triggered, this, &ARoguePlayerController::PrimaryInteract);

	// @todo: replace with Enhanced Input
	FInputActionBinding PauseBinding( "PauseMenu", IE_Pressed );
	PauseBinding.ActionDelegate.GetDelegateForManualSet().BindLambda( [this]()
	{
		Cast<ARogueHUD>(GetHUD())->TogglePauseMenu();
	});

	// Keeping as 'old' input for now until we figure out how to do this easily in Enhanced input
	InputComponent->BindAction("AnyKey", IE_Pressed, this, &ARoguePlayerController::AnyKeyInput);
}


void ARoguePlayerController::AnyKeyInput(FKey PressedKey)
{
	bIsUsingGamepad = PressedKey.IsGamepadKey();
}


void ARoguePlayerController::BeginPlayingState()
{
	BlueprintBeginPlayingState();
}


void ARoguePlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	OnPlayerStateReceived.Broadcast(PlayerState);
}
