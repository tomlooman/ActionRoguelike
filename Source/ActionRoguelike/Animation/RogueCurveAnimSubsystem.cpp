// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RogueCurveAnimSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueCurveAnimSubsystem)


void URogueCurveAnimSubsystem::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CurveAnimationsTick)
	
	// Curve Based Anims - Reverse to easily remove completed anims during iteration
	for (int i = ActiveAnims.Num() - 1; i >= 0; --i)
	{
		ActiveAnims[i].Tick(DeltaTime);

		FString DebugMsg = FString::Printf(TEXT("CurveAnim: %f"), ActiveAnims[i].Time);
		GEngine->AddOnScreenDebugMessage(100 + i, 0.0f, FColor::Green, DebugMsg);

		if (ActiveAnims[i].IsFinished())
		{
			ActiveAnims.RemoveAtSwap(i);
		}
	}

	// Easing Functions
	for (int i = ActiveEasingFuncs.Num() - 1; i >= 0; --i)
	{
		ActiveEasingFuncs[i].Tick(DeltaTime);
		
		FString DebugMsg = FString::Printf(TEXT("EasingAnim: %f"), ActiveEasingFuncs[i].Time);
		GEngine->AddOnScreenDebugMessage(200 + i, 0.0f, FColor::Green, DebugMsg);

		if (ActiveEasingFuncs[i].IsFinished())
		{
			ActiveEasingFuncs.RemoveAtSwap(i);
		}
	}
}


void URogueCurveAnimSubsystem::PlayCurveAnim(UCurveFloat* InCurveAsset, float InPlayRate, const TFunction<void (float CurrentValue)>& Func)
{
	check(InCurveAsset);

	ActiveAnims.Add(FActiveCurveAnim(InCurveAsset, Func, InPlayRate));
}


void URogueCurveAnimSubsystem::PlayEasingFunc(EEasingFunc::Type EasingType, float EasingExp, float InPlayRate, const TFunction<void(float)>& Func)
{
	// In prototype only supporting this one type...
	check(EasingType == EEasingFunc::EaseInOut);

	ActiveEasingFuncs.Add(FActiveEasingFunc(EasingExp, InPlayRate, Func));
}


TStatId URogueCurveAnimSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(RogueTweenSubsystem, STATGROUP_Tickables);
}
