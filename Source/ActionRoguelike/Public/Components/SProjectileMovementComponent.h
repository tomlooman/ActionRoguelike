// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "SProjectileMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	
	void Reset();

};
