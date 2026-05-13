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
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void TogglePauseMenu();
	
	URogueMainHUDWidget* GetMainHUD() const
	{
		return MainHUDInstance;
	}
	
protected:
	
	UFUNCTION()
	void OnPlayerStateReady(APlayerState* NewPlayerState);
	
	void CreateMainHUD();
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<URogueMainHUDWidget> MainHUDClass;

	UPROPERTY(BlueprintReadOnly, Transient)
	TObjectPtr<URogueMainHUDWidget> MainHUDInstance;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> PauseMenuInstance;

};
