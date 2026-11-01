// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RogueGameplayInterface.generated.h"

class URogueActionComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URogueGameplayInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ACTIONROGUELIKE_API IRogueGameplayInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	/* Called after the Actor state was restored from a SaveGame file. */
	UFUNCTION(BlueprintNativeEvent)
	void OnActorLoaded();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FText GetInteractText(AController* InstigatorController);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Interact(AController* InstigatorController);
	
	UFUNCTION(BlueprintNativeEvent)
	bool CanInteract();

	/*
	 * Play any cosmetic only effects such as particles, sounds, animations 
	 */
	UFUNCTION(BlueprintNativeEvent)
	void PlayInteractCosmetics();
	
	/*
	 * Stop playback on any predicted cosmetic effects
	 */
	UFUNCTION(BlueprintNativeEvent)
	void CancelInteractCosmetics();

	/*
	 * Retrieve the Data Asset for this Actor which defines important properties that sit outside of the main Blueprint
	 */
	virtual UDataAsset* GetActorConfigData() const { return nullptr; };
	
	/*
	 * Allow custom handling of impulses per Actor (eg. remap certain bones to others that result in better hit reactions or redirect to another Actor such as Corpses)
	 */
	virtual bool AddImpulseAtLocationCustom(FVector Impulse, FVector Location, FName BoneName = NAME_None)
	{
		return false;
	}
};
