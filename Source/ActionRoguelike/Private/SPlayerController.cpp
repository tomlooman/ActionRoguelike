// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"






void ASPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	OnPawnChanged.Broadcast(InPawn);
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
