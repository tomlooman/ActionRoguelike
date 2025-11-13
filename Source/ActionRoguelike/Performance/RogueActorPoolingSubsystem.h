// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueActorPoolingSubsystem.generated.h"

USTRUCT()
struct FActorPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> FreeActors;

	/* Tracking used actors, for debugging */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> InUseActors;
};

/**
 * General Actor Pooling Can Request and Release Actors from an (optionally) primed pool of Actors.
 */
UCLASS()
class ACTIONROGUELIKE_API URogueActorPoolingSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category= "Actor Pooling", meta = (WorldContext="WorldContextObject"))
	static AActor* SpawnActorPooled(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod SpawnHandling);

	static bool ReleaseToPool(AActor* Actor);

	/*
	 * Get dormant Actor from the pool, if none available or disabled we spawn a new Actor instance instead
	 */
	static AActor* AcquireFromPool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams);

	static bool IsPoolingEnabled();

	void PrimeActorPool(TSubclassOf<AActor> ActorClass, int32 Amount);

	AActor* AcquireFromPool_Internal(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams);

	bool ReleaseToPool_Internal(AActor* ActorToFree);

	void ActivateActor(AActor* InActor);

	void ParkActor(AActor* InActor);

protected:
	
	/*
	 * Tickable for debug rendering
	 */
	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

	/* Holds collection of available Actors, stored per class */
	UPROPERTY(Transient)
	TMap<TSubclassOf<AActor>, FActorPool> AvailableActorPool;
};
