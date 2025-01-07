// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RoguePlayerState.generated.h"

class ARoguePlayerState; // Forward declared to satisfy the delegate macros below
class URogueSaveGame;

// Event Handler for Credits
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreditsChanged, ARoguePlayerState*, PlayerState, int32, NewCredits, int32, Delta);
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

	UPROPERTY(Transient, EditDefaultsOnly, ReplicatedUsing="OnRep_Credits", Category = "Credits")
	int32 Credits;

	UPROPERTY(Transient, BlueprintReadOnly)
	float PersonalRecordTime;

	// OnRep_ can use a parameter containing the 'old value' of the variable it is bound to. Very useful in this case to figure out the 'delta'.
	UFUNCTION()
	void OnRep_Credits(int32 OldCredits);

public:

	/* Checks current record and only sets if better time was passed in. */
	UFUNCTION(BlueprintCallable)
	bool UpdatePersonalRecord(float NewTime);

	UFUNCTION(BlueprintCallable, Category = "Credits")
	int32 GetCredits() const;

	UFUNCTION(BlueprintCallable, Category = "Credits")
	void AddCredits(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Credits")
	bool TryRemoveCredits(int32 Delta);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCreditsChanged OnCreditsChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRecordTimeChanged OnRecordTimeChanged;

	UFUNCTION(BlueprintNativeEvent)
	void SavePlayerState(URogueSaveGame* SaveObject);

	UFUNCTION(BlueprintNativeEvent)
	void LoadPlayerState(URogueSaveGame* SaveObject);

};
