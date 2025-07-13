// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoguePickupActor.h"
#include "RoguePickupActor_HealthPotion.generated.h"


class UStaticMeshComponent;


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePickupActor_HealthPotion : public ARoguePickupActor
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditAnywhere, Category = "HealthPotion")
	int32 CreditCost = 50.0f;

public:
	virtual void Interact_Implementation(AController* InstigatorController) override;

	virtual FText GetInteractText_Implementation(AController* InstigatorController) override;
};
