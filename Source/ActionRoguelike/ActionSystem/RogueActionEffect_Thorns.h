// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueActionEffect.h"
#include "RogueActionEffect_Thorns.generated.h"


struct FAttributeModification;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueActionEffect_Thorns : public URogueActionEffect
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Thorns")
	float ReflectFraction = 0.2f;

	void OnHealthChanged(float NewValue, const FAttributeModification& AttributeModification);

	FDelegateHandle AttriChangedHandle;

public:
	virtual void StartAction_Implementation(AActor* Instigator) override;

	virtual void StopAction_Implementation(AActor* Instigator) override;
};
