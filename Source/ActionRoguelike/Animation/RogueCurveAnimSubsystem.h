// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueCurveAnimSubsystem.generated.h"


//DECLARE_DYNAMIC_DELEGATE_OneParam(FTweenAnimCallback, float, CurrentValue);


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

		// @todo: check when this is non-zero and how that should be handled
		Curve->GetTimeRange(CurrentTime, MaxTime);
	}

	UPROPERTY()
	UCurveFloat* Curve = nullptr;

	float PlayRate = 1.0f;

	/* Current time along curve */
	float CurrentTime = 0.0f;

	/* Cached max time to know when we finished */
	float MaxTime = 0.0f;

	// @todo: TFunction takes many bytes in the struct, we could replace it with a smaller delegate if we want to optimize this to be as small as possible
	// keeping this to look into at a later time
	TFunction<void(float)> Callback;

	void Tick(float DeltaTime)
	{
		CurrentTime += (DeltaTime*PlayRate);

		float CurrentValue = Curve->GetFloatValue(CurrentTime);

		Callback(CurrentValue);

		// Check if animation has completed
		// Remove on complete
		// if this was only anim playing, disable tick

		if (CurrentTime >= MaxTime)
		{
			// Mark as "Finished", will be cleaned up by subsystem
			Curve = nullptr;
		}
	}

	bool IsValid() const
	{
		return Curve != nullptr;
	}
};


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueCurveAnimSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
	TArray<FActiveCurveAnim> ActiveAnims;

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

public:

	// @todo: do I need this in BP, can I provide a different function for it so it has a nice 'on tick' exec pin
	// @todo: add loop and pingpong enum
	/* Start animation based on curve */
	//UFUNCTION(BlueprintCallable) 
	// eg. could just wrap that with the original delegate for BP calls.
	void PlayCurveAnim(UCurveFloat* InCurveAsset, float InPlayRate, const TFunction<void(float)>& Func);
};
