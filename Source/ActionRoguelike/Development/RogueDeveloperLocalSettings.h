// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "RogueDeveloperLocalSettings.generated.h"


/**
 * Place for Development Only configuration AND only stored via local INI files.
 */
UCLASS(Config=EditorPerProjectUserSettings)
class ACTIONROGUELIKE_API URogueDeveloperLocalSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:

	UPROPERTY(config, EditDefaultsOnly, Category = ActionRoguelike, meta = (ConsoleVariable = "game.DisableBotSpawning"))
	bool bDisableSpawnBotsOverride;

	virtual void PostInitProperties() override;
	
	virtual FName GetCategoryName() const override;
};
