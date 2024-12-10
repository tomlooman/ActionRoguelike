// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "RoguelikeDeveloperSettings.generated.h"

#if !UE_BUILD_SHIPPING
namespace DevelopmentOnly
{
	static bool bDisableSpawnBotsOverride = false;
	static FAutoConsoleVariableRef CVarSpawnBotsOverride(
	TEXT("game.DisableBotSpawning"),
	bDisableSpawnBotsOverride,
	TEXT("Disable spawning of bots for development only."));

}

#endif

/**
 * 
 */
UCLASS(Config=EditorPerProjectUserSettings)
class ACTIONROGUELIKE_API URoguelikeDeveloperSettings : public UDeveloperSettingsBackedByCVars
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
