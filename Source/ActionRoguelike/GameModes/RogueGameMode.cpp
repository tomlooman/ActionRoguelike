// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameMode.h"

#include "Player/RoguePlayerController.h"
#include "UI/RogueHUD.h"

ARogueGameMode::ARogueGameMode()
{
	PlayerControllerClass = ARoguePlayerController::StaticClass();
	HUDClass = ARogueHUD::StaticClass();
}
