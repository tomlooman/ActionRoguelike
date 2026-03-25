// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RogueCoinTestActor.generated.h"

UCLASS()
class ACTIONROGUELIKE_API ARogueCoinTestActor : public AActor
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
	void SpawnCoins(int32 SpawnCount);

protected:

	UPROPERTY(VisibleAnywhere, Category=Components)
	TObjectPtr<USceneComponent> DefaultSceneComp;

public:

	ARogueCoinTestActor();
};
