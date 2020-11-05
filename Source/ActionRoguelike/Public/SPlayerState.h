// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SPlayerState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCreditsChanged, ASPlayerState*, PlayerState, int32, NewCredits, int32, Delta);

class USSaveGame;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASPlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing="OnRep_Credits", Category = "Credits")
	int32 Credits;

	// OnRep_ can use a parameter containing the 'old value' of the variable it is bound to. Very useful in this case to figure out the 'delta'.
	UFUNCTION()
	void OnRep_Credits(int32 OldCredits);

	// Downside of using multicast here is that we send over more data over the net, since it's an RPC with two parameters. OnRep_ is "free" since Credits is already getting replicated anyway.
	//UFUNCTION(NetMulticast, Unreliable)
	//void MulticastCredits(float NewCredits, float Delta);

public:

	UFUNCTION(BlueprintCallable, Category = "Credits")
	int32 GetCredits() const;

	UFUNCTION(BlueprintCallable, Category = "Credits")
	void AddCredits(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "Credits")
	bool RemoveCredits(int32 Delta);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCreditsChanged OnCreditsChanged;

	UFUNCTION(BlueprintNativeEvent)
	void SavePlayerState(USSaveGame* SaveObject);

	UFUNCTION(BlueprintNativeEvent)
	void LoadPlayerState(USSaveGame* SaveObject);

};
