// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueDeferredTaskSystem.generated.h"

/*
 * Toggle between the testing code and experimental use cases easily
 */
#define USE_DEFERRED_TASKS 0


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueDeferredTaskSystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	static void AddTask(const UObject* WorldContextObject, TFunction<void()>&& InFunctionPtr);

	void AddFunctionTask(TFunction<void()>&& InFunctionPtr);

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

protected:
	
	TQueue<TFunction<void()>> FunctionPointers;

};
