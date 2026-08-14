// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RogueHUD.generated.h"

class URogueMainHUDWidget;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARogueHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	URogueMainHUDWidget* GetMainHUD() const
	{
		return MainWidgetInstance;
	}
	
protected:
	
	virtual void BeginPlay() override;
	
	// @todo: move to a PlayerDataAsset or similar shared location eventually
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<URogueMainHUDWidget> MainWidgetClass;
	
	UPROPERTY()
	TObjectPtr<URogueMainHUDWidget> MainWidgetInstance;
};
