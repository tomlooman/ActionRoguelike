// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RogueProjectileMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	
	void Reset();

};
