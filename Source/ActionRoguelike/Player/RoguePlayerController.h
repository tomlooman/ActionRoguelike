// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "RoguePlayerController.generated.h"

class UInputAction;
class URogueInteractionComponent;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<URogueInteractionComponent> InteractionComponent;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_Interact;

	void StartInteract();

	virtual void SetupInputComponent() override;

public:
	
	virtual FGenericTeamId GetGenericTeamId() const override;

	ARoguePlayerController();
};
