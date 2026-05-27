// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionEffect.h"

#include "ActionRoguelike.h"
#include "RogueActionSystemComponent.h"

void URogueActionEffect::StartAction_Implementation()
{
	Super::StartAction_Implementation();
	
	if (Duration > 0.0f)
	{
		ResetDuration();
	}
	
	if (Period > 0.0f)
	{
		// Looped
		GetWorld()->GetTimerManager().SetTimer(PeriodHandle, this,
			&ThisClass::ExecutePeriodicEffect, Period, true);
	}
}

void URogueActionEffect::StopAction_Implementation()
{
	// Run once more if we were about to tick the Period this frame
	if (Period > 0.0f && GetWorld()->GetTimerManager().GetTimerRemaining(PeriodHandle) < KINDA_SMALL_NUMBER)
	{
		ExecutePeriodicEffect();
	}
	
	Super::StopAction_Implementation();
	
	GetWorld()->GetTimerManager().ClearTimer(DurationHandle);
	GetWorld()->GetTimerManager().ClearTimer(PeriodHandle);
	// Alternatively clear all timers explicitly (will also clear any Blueprint added timers)
	//GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	
	GetOwningComponent()->RemoveAction(this);
}

void URogueActionEffect::ResetDuration()
{
	GetWorld()->GetTimerManager().SetTimer(DurationHandle, this, &ThisClass::StopAction, Duration);
}

void URogueActionEffect::IncrementStackSize()
{
	StackCount++;
	
	if (bResetDurationOnStackIncrease)
	{
		// Refresh duration each increment, on expiration all stacks are removed at once
		ResetDuration();
	}
	
	UE_LOG(LogGame, Log, TEXT("Incremented %s (%s) Stack to %d"), 
		*GetName(), 
		*GetNameSafe(GetOwningComponent()->GetOwner()), 
		StackCount);
}

void URogueActionEffect::ExecutePeriodicEffect_Implementation() { /* Nothing to do here */ }
