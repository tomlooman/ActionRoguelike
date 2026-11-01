// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetMathLibrary.h"
#include "Subsystems/WorldSubsystem.h"
#include "Curves/CurveFloat.h"
#include "RogueCurveAnimSubsystem.generated.h"



USTRUCT()
struct FActiveCurveAnim
{
	GENERATED_BODY()

	FActiveCurveAnim() {}

	FActiveCurveAnim(UCurveFloat* InCurve, TFunction<void (float)> InCallback, float InRate = 1.0f)
	{
		Curve = InCurve;
		Callback = InCallback;
		PlayRate = InRate;

		// We use first keyframe as start time, you could choose to always start at 0.0f
		Curve->GetTimeRange(Time, MaxTime);
	}

	UPROPERTY()
	TObjectPtr<UCurveFloat> Curve = nullptr;

	float PlayRate = 1.0f;

	/* Current time along curve */
	float Time = 0.0f;

	/* Cached max time to know when we finished */
	float MaxTime = 0.0f;

	TFunction<void(float)> Callback;
	
	void Tick(float DeltaTime)
	{
		// Clamp to avoid overshoots
		Time = FMath::Min (Time + (DeltaTime * PlayRate), MaxTime);

		float CurrentValue = Curve->GetFloatValue(Time);
		Callback(CurrentValue);

		if (Time >= MaxTime)
		{
			// Mark as "Finished", will be cleaned up by subsystem
			Curve = nullptr;
		}
	}
	
	void Reset()
	{
		// Resets time to start
		Curve->GetTimeRange(Time, MaxTime);
		
		// Run once on the original frame
		float CurrentValue = Curve->GetFloatValue(Time);
		Callback(CurrentValue);
	}

	bool IsFinished() const
	{
		return Curve == nullptr;
	}
};

struct FCurveAnimHandle
{
	uint64 Id = 0;

	bool IsValid() const { return Id != 0; }
	void Reset() { Id = 0; }

	bool operator==(const FCurveAnimHandle& Other) const
	{
		return Id == Other.Id;
	}
	
	friend uint32 GetTypeHash(const FCurveAnimHandle& Handle)
	{
		return ::GetTypeHash(Handle.Id);
	}
};


USTRUCT()
struct FActiveEasingFunc
{
	GENERATED_BODY()

	FActiveEasingFunc() {}

	FActiveEasingFunc(float InEasingExp, float InPlayRate, TFunction<void (float)> InCallback)
		: EasingExp(InEasingExp), PlayRate(InPlayRate), Callback(InCallback) 	{}

	float EasingExp = 2.0f;
	
	float PlayRate = 1.0f;
	
	float Time = 0.0f;

	TFunction<void(float)> Callback;

	void Tick(float DeltaTime)
	{
		Time += (DeltaTime*PlayRate);

		// Clamp
		Time = FMath::Min(Time, 1.0f);

		float CurrentValue = FMath::InterpEaseInOut(0.0f, 1.0f, Time, EasingExp);

		Callback(CurrentValue);
	}

	bool IsFinished()
	{
		// @todo: allow more clear anim duration
		return Time >= 1.0f;;
	}
};


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueCurveAnimSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	// This could be a FInstancedStruct array if we want a variety of struct options with different sets of data
	TMap<FCurveAnimHandle, FActiveCurveAnim> ActiveAnims;

	TArray<FActiveEasingFunc> ActiveEasingFuncs;
	
	uint64 NextHandleId = 1;

public:

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

	/* Start animation based on curve */
	FCurveAnimHandle PlayCurveAnim(UCurveFloat* InCurveAsset, float InPlayRate, const TFunction<void(float)>& Func);
	
	bool StopCurveAnim(const FCurveAnimHandle& InHandle);
	
	bool CancelCurveAnim(const FCurveAnimHandle& InHandle);

	/* Play easing function with output between 0.0 - 1.0 */
	void PlayEasingFunc(EEasingFunc::Type EasingType, float EasingExp, float InPlayRate, const TFunction<void(float)>& Func);
	
	float GetAnimationEndValue(UCurveFloat* InCurveAsset);
	
	float GetAnimationStartValue(UCurveFloat* InCurveAsset);
	
	float GetAnimDuration(UCurveFloat* InCurveAsset) const;
};
