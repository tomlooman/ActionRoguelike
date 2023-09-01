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

	bool ReleaseToPool(AActor* Actor);

	AActor* AquireFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams);

	/* Helper function to easily spawn or acquire a pooled actor, replaces the use of GetWorld()->SpawnActor() */
	static AActor* GetPooledActor(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams);


protected:

	/* Holds collection of available Actors, stored per class */
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FActorPool> AvailableActorPool;
};
