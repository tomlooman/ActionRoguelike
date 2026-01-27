// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAction.h"

#include "RogueActionSystemComponent.h"



void URogueAction::StartAction()
{
	float GameTime = 0.0f;
	//GetWorld()->TimeSeconds;
	
	UE_LOGFMT(LogTemp, Log, "Started Action {ActionName} - {WorldTime}",
		("ActionName", ActionName),
		("WorldTime", GameTime));
}

URogueActionSystemComponent* URogueAction::GetOwningComponent() const
{
	return Cast<URogueActionSystemComponent>(GetOuter());
}