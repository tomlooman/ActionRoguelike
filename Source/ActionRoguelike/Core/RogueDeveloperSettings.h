// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RogueDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig)
class ACTIONROGUELIKE_API URogueDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	
	/* Pickup Subsystem - Coin Mesh used in ISMs */
	UPROPERTY(Config, EditDefaultsOnly, Category = "PickupSubsystem")
	TSoftObjectPtr<UStaticMesh> PickupCoinMesh;
	
	virtual FName GetCategoryName() const override;
};
