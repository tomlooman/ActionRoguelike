// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SActorPoolingSubsystem.generated.h"

USTRUCT()
struct FActorPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<AActor>> FreeActors;
};

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USActorPoolingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category= "Actor Pooling", meta = (WorldContext="WorldContextObject"))
	static AActor* SpawnActorPooled(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod SpawnHandling);

	static bool ReleaseToPool(AActor* Actor);
	
	static AActor* AcquireFromPool(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams);

	static bool IsPoolingEnabled(const UObject* WorldContextObject);

	void PrimeActorPool(TSubclassOf<AActor> ActorClass, int32 Amount);

protected:
	
	AActor* AcquireFromPool_Internal(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams);

	bool ReleaseToPool_Internal(AActor* Actor);

protected:

	/* Holds collection of available Actors, stored per class */
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FActorPool> AvailableActorPool;
};
