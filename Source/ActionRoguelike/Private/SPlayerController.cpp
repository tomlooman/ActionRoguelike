// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"

#include "Development/RogueCheatManager.h"
#include "UI/RogueHUD.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SPlayerController)



ASPlayerController::ASPlayerController()
{
	CheatClass = URogueCheatManager::StaticClass();
	bIsUsingGamepad = false;
}


void ASPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// @todo: replace with Enhanced Input
	FInputActionBinding PauseBinding( "PauseMenu", IE_Pressed );
	PauseBinding.ActionDelegate.GetDelegateForManualSet().BindLambda( [this]()
	{
		Cast<ARogueHUD>(GetHUD())->TogglePauseMenu();
	});

	// Keeping as 'old' input for now until we figure out how to do this easily in Enhanced input
	InputComponent->BindAction("AnyKey", IE_Pressed, this, &ASPlayerController::AnyKeyInput);


}


void ASPlayerController::AnyKeyInput(FKey PressedKey)
{
	bIsUsingGamepad = PressedKey.IsGamepadKey();
}


void ASPlayerController::BeginPlayingState()
{
	BlueprintBeginPlayingState();
}


void ASPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	OnPlayerStateReceived.Broadcast(PlayerState);
}
