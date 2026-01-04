// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RoguePickupSubsystem.generated.h"

class UInstancedStaticMeshComponent;

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
	
	void AddCreditsPickup(FVector Origin, int32 CreditAmount);

	TArray<FPrimitiveInstanceId> AddMeshInstances(const TArray<FTransform>& InAdded);

	void RemoveMeshInstances(const TArray<FPrimitiveInstanceId>& IdsToRemove);

protected:
	
	void RemoveCreditsPickup(int32 InIndex);

	// -- These arrays are in sync
	TArray<FVector> CreditPickupLocations;
	TArray<int32> CreditPickupAmount;
	TArray<FPrimitiveInstanceId> MeshIDs;
	// -- end

	FPrimitiveInstanceId AddMeshInstance(FVector InLocation);

	void CreateWorldISM();

	/* Single ISM that holds all coins, registered directly with the world instead of Actor wrapper */
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> WorldISM;

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(URoguePickupSubsystem, STATGROUP_Tickables);
	}
};
