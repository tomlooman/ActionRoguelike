// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoguePickupActor.h"
#include "RoguePickupActor_Credits.generated.h"


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePickupActor_Credits : public ARoguePickupActor
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, Category = "Credits")
	int32 CreditsAmount;

public:
	virtual void Interact_Implementation(AController* InstigatorController) override;

	ARoguePickupActor_Credits();
};
