// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueAction.h"
#include "RogueAction_DropLoot.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueAction_DropLoot : public URogueAction
{
	GENERATED_BODY()
	
protected:
	
	/* Max number (between 1 and X) of credits to contain within a single coin spawned in the world */
	UPROPERTY(EditDefaultsOnly, Category="Loot")
	int32 MaxCreditsPerCoin = 15;
	
	UPROPERTY(EditDefaultsOnly, Category="Loot")
	float MaxDropDistance = 250;
	
public:
	
	URogueAction_DropLoot();
	
	virtual void StartAction_Implementation() override;
};
