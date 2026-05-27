// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueAction.h"
#include "RogueActionEffect.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueActionEffect : public URogueAction
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	float Duration = 0.0f;
	
	/* Time between 'ticks' to apply effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Effect)
	float Period = 0.0f;
	
	/* Adding the same Effect multiple times instead increases the stack size. Can be used to amplify certain effects. 
	 * Resets effect duration unless bResetDurationOnStackIncrease is false  */
	UPROPERTY(Transient, BlueprintReadOnly, Category=Effect)
	int32 StackCount = 1;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Effect)
	bool bResetDurationOnStackIncrease = true;
	
	FTimerHandle DurationHandle;
	FTimerHandle PeriodHandle;
	
	UFUNCTION(BlueprintNativeEvent, Category = "Effect")
	void ExecutePeriodicEffect();
	
	void ResetDuration();
	
public:
	
	void IncrementStackSize();
	
	virtual void StartAction_Implementation() override;
	
	virtual void StopAction_Implementation() override;
};
