// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RogueHUD.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"


void ARogueHUD::TogglePauseMenu()
{
	APlayerController* MyPC = GetOwningPlayerController();
	
	if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
	{
		PauseMenuInstance->RemoveFromParent();
		PauseMenuInstance = nullptr;

		MyPC->bShowMouseCursor = false;
		MyPC->SetInputMode(FInputModeGameOnly());

		// Single-player only
		if (GetWorld()->IsNetMode(NM_Standalone))
		{
			UGameplayStatics::SetGamePaused(this, false);
		}

		return;
	}

	PauseMenuInstance = CreateWidget<UUserWidget>(MyPC, PauseMenuClass);
	if (PauseMenuInstance)
	{
		PauseMenuInstance->AddToViewport(100);

		MyPC->bShowMouseCursor = true;
		MyPC->SetInputMode(FInputModeUIOnly());

		// Single-player only
		if (GetWorld()->IsNetMode(NM_Standalone))
		{
			UGameplayStatics::SetGamePaused(this, true);
		}
	}
}
