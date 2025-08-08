// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameMode.h"

#include "ActionRoguelike/Player/RoguePlayerController.h"

ARogueGameMode::ARogueGameMode()
{
	PlayerControllerClass = ARoguePlayerController::StaticClass();
}
