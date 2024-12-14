// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueTweenSubsystem.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FTweenAnimCallback, float, CurrentValue);


USTRUCT()
struct FActiveTweenData
{
	GENERATED_BODY()

	FActiveTweenData() {}

	FActiveTweenData(UCurveFloat* InCurve, TFunction<void (float)> InCallback, float InRate = 1.0f)
	{
		Curve = InCurve;
		Callback = InCallback;
		PlayRate = InRate;

		// @todo: check when this is non-zero and how that should be handled
		Curve->GetTimeRange(CurrentTime, MaxTime);
	}

	UPROPERTY()
	UCurveFloat* Curve = nullptr;

	float PlayRate = 1.0f;
	
	TFunction<void(float)> Callback;
	//FTweenAnimCallback Callback;

	/* Current time along curve */
	float CurrentTime = 0.0f;

	/* Cached max time to know when we finished */
	float MaxTime = 0.0f;
};


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueTweenSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

	// This will be an array
	FActiveTweenData CurrentAnimation;

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

	// @todo: maybe just make 2 overloads to handle Vector and float rather than rely on CurveBase

public:
	
	/* Start animation based on curve */  // add loop and pingpong enum
	//UFUNCTION(BlueprintCallable) // @todo: do I need this in BP, can I provide a different function for it so it has a nice 'on tick' exec pin
	// eg. could just wrap that with the original delegate for BP calls.
	void PlayTween(UCurveFloat* InCurveAsset, float InPlayRate, TFunction<void(float)> Func); 
};
