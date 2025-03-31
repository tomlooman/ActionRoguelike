// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "RogueDeveloperSettings.generated.h"


/**
 * 
 */
UCLASS(Config=EditorPerProjectUserSettings)
class ACTIONROGUELIKE_API URogueDeveloperSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:

	UPROPERTY(config, EditDefaultsOnly, Category = ActionRoguelike, meta = (ConsoleVariable = "game.DisableBotSpawning"))
	bool bDisableSpawnBotsOverride;

	//~UDeveloperSettings interface
	virtual void PostInitProperties() override;
	
	virtual FName GetCategoryName() const override;
	//~End of UDeveloperSettings interface
};
