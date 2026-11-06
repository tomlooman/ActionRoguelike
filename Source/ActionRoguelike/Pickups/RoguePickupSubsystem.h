// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RoguePickupSubsystem.generated.h"

class UInstancedStaticMeshComponent;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URoguePickupSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	
	void AddCoinsPickup(const TArray<FVector>& Locations, const TArray<int32>& CoinAmount);

	TArray<FPrimitiveInstanceId> AddMeshInstances(const TArray<FTransform>& InAdded);

	void RemoveMeshInstances(const TArray<FPrimitiveInstanceId>& IdsToRemove);

	/* Local feedback invoked by the collecting player's Client RPC. */
	void PlayLocalCoinPickupSound();

protected:
	
	void RemoveCoinsPickup(int32 InIndex);

	// -- These arrays are in sync
	TArray<FVector> CoinPickupLocations;
	TArray<int32> CoinCreditRewards;
	TArray<FPrimitiveInstanceId> CoinMeshIDs;
	// -- end

	FPrimitiveInstanceId AddMeshInstance(FVector InLocation);

	void CreateWorldISM();

	/* Single ISM that holds all coins, registered directly with the world instead of Actor wrapper */
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> WorldISM;
	
	UPROPERTY()
	TObjectPtr<UAudioComponent> CoinPickupAudioComp;

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(URoguePickupSubsystem, STATGROUP_Tickables);
	}

	virtual bool IsTickable() const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void OnSoundAssetLoadComplete(const FSoftObjectPath& SoftObjectPath, UObject* LoadedObject);
};
