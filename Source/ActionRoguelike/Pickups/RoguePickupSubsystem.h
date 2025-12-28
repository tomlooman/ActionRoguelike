// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RoguePickupSubsystem.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;

// Easy toggle to remove the use case testing from game code
#define USE_DOD_CREDIT_PICKUPS 1

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URoguePickupSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	
	void AddNewCreditsPickup(FVector Origin, int32 CreditAmount);

protected:
	
	void RemoveCreditsPickup(int32 InIndex);
	
	TArray<FVector> CreditPickupLocations;

	TArray<int32> CreditPickupAmount;

	TArray<bool> CreditsPickupDebugList;

	/* Active VFX mapped to CreditsPickupDataList */
	UPROPERTY()
	TArray<UNiagaraComponent*> CreditFXList;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> CoinPickupFXTemplate;

	UPROPERTY(EditAnywhere)
	float PickupRadius = 200;

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(URoguePickupSubsystem, STATGROUP_Tickables);
	}
};
