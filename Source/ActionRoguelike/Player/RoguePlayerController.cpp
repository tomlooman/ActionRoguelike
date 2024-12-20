// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePlayerController.h"

#include "Development/RogueCheatManager.h"
#include "UI/RogueHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePlayerController)



ARoguePlayerController::ARoguePlayerController()
{
	CheatClass = URogueCheatManager::StaticClass();
	bIsUsingGamepad = false;
}


void ARoguePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

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
