// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/RogueTweenSubsystem.h"


void URogueTweenSubsystem::Tick(float DeltaTime)
{
	if (CurrentAnimation.Curve == nullptr)
	{
		// for now skip until we assigned a curve
		return;
	}
	
	CurrentAnimation.CurrentTime += DeltaTime;

	float CurrentValue = CurrentAnimation.Curve->GetFloatValue(CurrentAnimation.CurrentTime);

	CurrentAnimation.Callback(CurrentValue);

	// Check if animation has completed
		// Remove on complete
		// if this was only anim playing, disable tick

	if (CurrentAnimation.CurrentTime >= CurrentAnimation.MaxTime)
	{
		// Mark as "Finished"
		CurrentAnimation.Curve = nullptr;

		// Remove from array
	}
}


void URogueTweenSubsystem::PlayTween(UCurveFloat* InCurveAsset, float InPlayRate, TFunction<void (float CurrentValue)> Func /*const FTweenAnimCallback& Callback*/)
{
	// @todo: if valid, pass into array and start playing.
	CurrentAnimation = FActiveTweenData(InCurveAsset, Func, InPlayRate);

	// @todo: activate tick if this was first entry into the array
	
}


TStatId URogueTweenSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(RogueTweenSubsystem, STATGROUP_Tickables);
}