// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueCoinPickupSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueCoinPickupSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	void AddCoinPickups(TArray<FVector> NewLocations, TArray<int32> NewAmounts);

	void RemoveCoinPickup(int32 IndexToRemove);

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(RogueCoinPickupSubsystem, STATGROUP_Tickables);
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

protected:

	void OnPickupMeshLoadComplete(const FSoftObjectPath& SoftObjectPath, UObject* LoadedObject);

	void OnPickupSoundLoadComplete(const FSoftObjectPath& SoftObjectPath, UObject* LoadedObject);

	void PlayPickupSound();

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> WorldISM;

	UPROPERTY()
	TObjectPtr<UAudioComponent> WorldAudioComp;

	/* Cached Param from Developer Settings for Audio Comp Pickups */
	FName CoinPickupTriggerParamName;
	
	TArray<FVector> CoinLocations;
	TArray<int32> CoinAmounts;
	TArray<FPrimitiveInstanceId> MeshIDs;
};
