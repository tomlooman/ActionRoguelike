// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/RogueCurveAnimSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueCurveAnimSubsystem)


void URogueCurveAnimSubsystem::Tick(float DeltaTime)
{
	/*
	for (FActiveCurveAnim& Anim : ActiveAnims)
	{
		Anim.Tick(DeltaTime);

		// once nulled, we are 'finished'
		if (Anim.Curve == nullptr)
		{
			
		}
	}*/

	for (int i = ActiveAnims.Num() - 1; i >= 0; --i)
	{
		ActiveAnims[i].Tick(DeltaTime);

		if (ActiveAnims[i].Curve == nullptr)
		{
			ActiveAnims.RemoveAt(i);
		}
	}
}


void URogueCurveAnimSubsystem::PlayCurveAnim(UCurveFloat* InCurveAsset, float InPlayRate, const TFunction<void (float CurrentValue)>& Func)
{
	check(InCurveAsset);

	ActiveAnims.Add(FActiveCurveAnim(InCurveAsset, Func, InPlayRate));
}


TStatId URogueCurveAnimSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(RogueTweenSubsystem, STATGROUP_Tickables);
}
