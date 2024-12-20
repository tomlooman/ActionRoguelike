// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/RogueAction.h"
#include "RogueAction_MinionRangedAttack.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueAction_MinionRangedAttack : public URogueAction
{
	GENERATED_BODY()

	/* Max Random Bullet Spread (in Degrees) in positive and negative angle (shared between Yaw and Pitch) */
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxBulletSpread = 2.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	TSubclassOf<AActor> ProjectileClass;

	virtual void StartAction_Implementation(AActor* Instigator) override;
};
