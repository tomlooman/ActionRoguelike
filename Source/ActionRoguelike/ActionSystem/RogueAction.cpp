// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAction.h"

#include "RogueActionSystemComponent.h"



void URogueAction::StartAction_Implementation()
{
	float GameTime = GetWorld()->TimeSeconds;
	
	UE_LOGFMT(LogTemp, Log, "Started Action {ActionName} - {WorldTime}",
		("ActionName", ActionName),
		("WorldTime", GameTime));
}

void URogueAction::StopAction_Implementation()
{
	float GameTime = GetWorld()->TimeSeconds;
	
	UE_LOGFMT(LogTemp, Log, "Stopped Action {ActionName} - {WorldTime}",
		("ActionName", ActionName),
		("WorldTime", GameTime));
}

URogueActionSystemComponent* URogueAction::GetOwningComponent() const
{
	return Cast<URogueActionSystemComponent>(GetOuter());
}
