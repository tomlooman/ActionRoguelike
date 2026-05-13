// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RogueHUD.h"

#include "RogueMainHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Player/RoguePlayerController.h"


void ARogueHUD::BeginPlay()
{
	Super::BeginPlay();
	
	ARoguePlayerController* MyPC = Cast<ARoguePlayerController>(GetOwningPlayerController());
	
	// For clients we wait until the playerstate is ready
	if (GetNetMode() == NM_Client && MyPC->PlayerState == nullptr)
	{
		MyPC->OnPlayerStateReceived.AddDynamic(this, &ThisClass::OnPlayerStateReady);
	}
	else
	{
		// For host or standalone, create immediately
		CreateMainHUD();
	}
}


void ARogueHUD::OnPlayerStateReady(APlayerState* NewPlayerState)
{
	CreateMainHUD();
}


void ARogueHUD::CreateMainHUD()
{
	if (MainHUDClass)
	{
		APlayerController* MyPC = GetOwningPlayerController();
		MainHUDInstance = CreateWidget<URogueMainHUDWidget>(MyPC, MainHUDClass);
		MainHUDInstance->AddToViewport();
	}
}


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
