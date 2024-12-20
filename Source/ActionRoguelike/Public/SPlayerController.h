// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStateChanged, APlayerState*, NewPlayerState);


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category= "Input")
	bool IsUsingGamepad() const
	{
		return bIsUsingGamepad;
	}

protected:

	virtual void SetupInputComponent() override;

	void AnyKeyInput(FKey PressedKey);
	
	// Listen for incoming player state (for clients this may be nullptr when initially joining a game, 
	// afterwards player state will not change again as PlayerControllers maintain the same player state throughout the level)
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStateChanged OnPlayerStateReceived;

	/* Called when player controller is ready to begin playing, good moment to initialize things like UI which might be too early in BeginPlay 
		(esp. in multiplayer clients where not all data such as PlayerState may have been received yet) */
	virtual void BeginPlayingState() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BlueprintBeginPlayingState();

	virtual void OnRep_PlayerState() override;
	
private:

	/* Was any input recently using GamePad */
	bool bIsUsingGamepad;

public:

	ASPlayerController();
};
