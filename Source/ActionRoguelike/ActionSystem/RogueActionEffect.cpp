// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionEffect.h"

#include "RogueActionSystemComponent.h"

void URogueActionEffect::StartAction_Implementation()
{
	Super::StartAction_Implementation();
	
	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(DurationHandle, this, &ThisClass::StopAction, Duration);
	}
}

void URogueActionEffect::StopAction_Implementation()
{
	Super::StopAction_Implementation();
	
	GetWorld()->GetTimerManager().ClearTimer(DurationHandle);
	
	GetOwningComponent()->RemoveAction(this);
}
