// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueActionEffect.h"
#include "RogueActionEffect_Overwhelm.generated.h"

/**
 * Tracks recent damage received and applies Stun debuff when a threshold is reached.
 */
UCLASS()
class ACTIONROGUELIKE_API URogueActionEffect_Overwhelm : public URogueActionEffect
{
	GENERATED_BODY()
	
protected:
	
	// Amount of damage required (in one hit or summed) to apply the Debuff effect
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	float DamageThreshold = 20.0f;
	
	// The Debuff to apply when the damage threshold is reached (eg. apply Stunned)
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	TSubclassOf<URogueActionEffect> EffectOnThreshold;
	
	// Damage within this timeframe is summed to potentially reach the damage threshold
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	float TimeDeltaThreshold = 1.0f;

	float LastDamageTime = 0.0f;
	
	float SummedRecentDamage = 0.0f;
	
	FDelegateHandle HealthDelegateHandle;
	
	UFUNCTION()
	void OnHealthChanged(FGameplayTag AttributeTag, float NewAttributeValue, float OldAttributeValue);
	
public:
	
	virtual void StartAction_Implementation() override;
	
	virtual void StopAction_Implementation() override;
	
	URogueActionEffect_Overwhelm();
};
