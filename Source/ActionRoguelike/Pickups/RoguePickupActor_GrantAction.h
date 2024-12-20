// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoguePickupActor.h"
#include "RoguePickupActor_GrantAction.generated.h"

class URogueAction;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePickupActor_GrantAction : public ARoguePickupActor
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, Category = "Powerup")
	TSubclassOf<URogueAction> ActionToGrant;

public:
	virtual void Interact_Implementation(APawn* InstigatorPawn) override;

};
