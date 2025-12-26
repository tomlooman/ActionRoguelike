// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueDeferredTaskSystem.generated.h"

/*
 * Toggle between the testing code and experimental use cases easily
 */
#define USE_DEFERRED_TASKS 1


DECLARE_DYNAMIC_DELEGATE(FDeferredTaskDelegate);


USTRUCT()
struct FDeferredTask
{
	GENERATED_BODY()

	TFunction<void()> FunctionPtr;

	FDeferredTaskDelegate Delegate;
};

/**
 * Allow functions or "tasks" to be delayed to later in the frame or a follow game frame when "budget" is available.
 * Goal is to improve frame pacing by spreading out non-critical functionality that can be safely delayed across multiple frames
 * without complex bookkeeping.
 *
 * Delegate hook is added as an example of letting Blueprint make use of this in a very straightforward way.
 * With C++ you instead pass lambdas into the system.
 *
 * The system ticks after most TickGroups, before TG_PostUpdateWork.
 */
UCLASS()
class ACTIONROGUELIKE_API URogueDeferredTaskSystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	static void AddLambda(const UObject* WorldContextObject, TFunction<void()> InFunctionPtr);

	void AddFunction(TFunction<void()> InFunctionPtr);
	
	/* Blueprint-way of delaying some task while budget is available */
	UFUNCTION(BlueprintCallable)
	void AddDelegate(FDeferredTaskDelegate InDelegate);

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

protected:

	double FrameStartTime;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	TQueue<FDeferredTask> FunctionPointers;
};
