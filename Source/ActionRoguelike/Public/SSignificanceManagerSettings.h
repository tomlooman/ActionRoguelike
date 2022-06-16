// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SSignificanceData.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "SSignificanceManagerSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta = (DisplayName="Significance Manager"))
class ACTIONROGUELIKE_API USSignificanceManagerSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:

	USSignificanceData* GetConfig() const;
	
	UPROPERTY(Config, EditAnywhere, Category = "General")
	bool bEnableSingificanceManager;

	/* Contains per class/type significance settings - held in DataAsset to allow runtime tweaking while in editor. */
	UPROPERTY(Config, EditAnywhere, Category = "General")
	TSoftObjectPtr<USSignificanceData> ConfigurationAsset;

	USSignificanceManagerSettings();
};
