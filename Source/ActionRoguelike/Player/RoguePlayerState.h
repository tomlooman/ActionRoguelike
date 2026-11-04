// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RoguePlayerState.generated.h"

class ARoguePlayerState; // Forward declared to satisfy the delegate macros below
class URogueSaveGame;

// Event Handler for Personal Record Time
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRecordTimeChanged, ARoguePlayerState*, PlayerState, float, NewTime, float, OldRecord);

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(Transient, BlueprintReadOnly)
	float PersonalRecordTime;

public:

	/* Checks current record and only sets if better time was passed in. */
	UFUNCTION(BlueprintCallable)
	bool UpdatePersonalRecord(float NewTime);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRecordTimeChanged OnRecordTimeChanged;

	UFUNCTION(BlueprintNativeEvent)
	void SavePlayerState(URogueSaveGame* SaveObject);

	UFUNCTION(BlueprintNativeEvent)
	void LoadPlayerState(URogueSaveGame* SaveObject);

};
