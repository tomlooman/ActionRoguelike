// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueProjectile.h"
#include "RogueProjectile_Dash.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARogueProjectile_Dash : public ARogueProjectile
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportDelay = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float DetonateDelay = 0.2f;

	// Handle to cancel timer if we already hit something
	FTimerHandle TimerHandle_DelayedDetonate;

	void TeleportInstigator();

	virtual void BeginPlay() override;
	
	UPROPERTY(Replicated)
	float TeleportAtTime;

public:
	
	// Base class using BlueprintNativeEvent, we must override the _Implementation not the Explode()
	virtual void Explode_Implementation() override;

	ARogueProjectile_Dash();
};
