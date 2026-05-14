// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueActionEffect.h"
#include "RogueActionEffect_StunFromDamage.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ACTIONROGUELIKE_API URogueActionEffect_StunFromDamage : public URogueActionEffect
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	float DamageThreshold = 20.0f;
	
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	TSubclassOf<URogueActionEffect> EffectOnThreshold;
	
	UPROPERTY(EditDefaultsOnly, Category=Effect)
	float TimeDeltaThreshold = 1.0f;
	
	float LastDamageTime = 0.0f;
	
	float SummedRecentDamage = 0.0f;
	
	UFUNCTION()
	void OnHealthChanged(float NewValue, const FAttributeModification& AttriMod);
	
public:
	
	virtual void StartAction_Implementation(AActor* Instigator) override;
};
