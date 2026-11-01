// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RogueCurveAnimSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueCurveAnimSubsystem)


void URogueCurveAnimSubsystem::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CurveAnimationsTick)
	
	// Curve Based Anims
	int32 DebugKey = 0;
	for (auto It = ActiveAnims.CreateIterator(); It; ++It)
	{
		FActiveCurveAnim& Anim = It.Value();
		Anim.Tick(DeltaTime);

		FString DebugMsg = FString::Printf(TEXT("CurveAnim: %f"), Anim.Time);
		GEngine->AddOnScreenDebugMessage(100 + (DebugKey++), 0.0f, FColor::Green, DebugMsg);

		if (Anim.IsFinished())
		{
			It.RemoveCurrent();
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


FCurveAnimHandle URogueCurveAnimSubsystem::PlayCurveAnim(UCurveFloat* InCurveAsset, float InPlayRate, const TFunction<void (float CurrentValue)>& Func)
{
	check(InCurveAsset);

	FCurveAnimHandle Handle;
	Handle.Id = NextHandleId++;
	
	ActiveAnims.Add(Handle, FActiveCurveAnim(InCurveAsset, Func, InPlayRate));
	
	return Handle;
}


bool URogueCurveAnimSubsystem::StopCurveAnim(const FCurveAnimHandle& InHandle)
{
	return ActiveAnims.Remove(InHandle) > 0;
}


bool URogueCurveAnimSubsystem::CancelCurveAnim(const FCurveAnimHandle& InHandle)
{
	FActiveCurveAnim* Anim = ActiveAnims.Find(InHandle);
	// Reset to original state, triggers anim at start time once
	if (Anim)
	{
		Anim->Reset();
		ActiveAnims.Remove(InHandle);
		return true;
	}
	
	return false;
}


void URogueCurveAnimSubsystem::PlayEasingFunc(EEasingFunc::Type EasingType, float EasingExp, float InPlayRate, const TFunction<void(float)>& Func)
{
	// In prototype only supporting this one type...
	check(EasingType == EEasingFunc::EaseInOut);

	ActiveEasingFuncs.Add(FActiveEasingFunc(EasingExp, InPlayRate, Func));
}

float URogueCurveAnimSubsystem::GetAnimationEndValue(UCurveFloat* InCurveAsset)
{
	if (InCurveAsset)
	{
		float MinTime;
		float MaxTime;
		InCurveAsset->GetTimeRange(MinTime, MaxTime);
		
		return InCurveAsset->GetFloatValue(MaxTime);
	}
	
	return 0.0f;
}

float URogueCurveAnimSubsystem::GetAnimationStartValue(UCurveFloat* InCurveAsset)
{
	if (InCurveAsset)
	{
		float MinTime;
		float MaxTime;
		InCurveAsset->GetTimeRange(MinTime, MaxTime);
		
		return InCurveAsset->GetFloatValue(MinTime);
	}
	
	return 0.0f;
}

float URogueCurveAnimSubsystem::GetAnimDuration(UCurveFloat* InCurveAsset) const
{
	if (InCurveAsset)
	{
		float MinTime;
		float MaxTime;
		InCurveAsset->GetTimeRange(MinTime, MaxTime);
		
		return MaxTime - MinTime;
	}
	
	return 0.0f;
}


TStatId URogueCurveAnimSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(RogueTweenSubsystem, STATGROUP_Tickables);
}
