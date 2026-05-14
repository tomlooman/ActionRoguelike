// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidgetPool.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueActorPoolingSubsystem.generated.h"

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
class ACTIONROGUELIKE_API URogueActorPoolingSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category= "Actor Pooling")
	AActor* SpawnActorPooled(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod SpawnHandling);

	bool ReleaseToPool(AActor* Actor);

	template <class T>
	T* AcquireFromPool(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams = FActorSpawnParameters())
	{
		return Cast<T>(AcquireFromPool_Internal(ActorClass, SpawnTransform, SpawnParams));
	}

	static bool IsPoolingEnabled(const UObject* WorldContextObject);

	void PrimeActorPool(TSubclassOf<AActor> ActorClass, int32 Amount);
	
	//UPROPERTY(Transient)
	//FUserWidgetPool WidgetPool;

protected:

	AActor* AcquireFromPool_Internal(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, FActorSpawnParameters SpawnParams);

	bool ReleaseToPool_Internal(AActor* Actor);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

protected:

	/* Holds collection of available Actors, stored per class */
	UPROPERTY(Transient)
	TMap<TSubclassOf<AActor>, FActorPool> AvailableActorPool;
};
