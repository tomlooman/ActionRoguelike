// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "STickablesSubsystem.generated.h"


/* Registered tick of the subsystem with the standard tick task graph */
USTRUCT()
struct FTickablesTickFunction : public FTickFunction
{
	GENERATED_BODY()

	UPROPERTY()
	USTickablesSubsystem* Target = nullptr;

	virtual void ExecuteTick(float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;

	virtual FString DiagnosticMessage() override {return TEXT("FAggregateTickFunction"); }
};

template <>
struct TStructOpsTypeTraits<FTickablesTickFunction> : TStructOpsTypeTraitsBase2<FTickablesTickFunction>
{
	enum
	{
		WithCopy = false
	};
};


/**
 * 
 */
UCLASS() // @todo: rename to clear be TickManager
class ACTIONROGUELIKE_API USTickablesSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	
	void RegisterComponent(FActorComponentTickFunction* TickFunction);
	
	void DeRegisterComponent(FActorComponentTickFunction* TickFunction);
	
	void ExecuteTick(ETickingGroup TickGroup, float DeltaTime, ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent);

protected:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	TArray<FActorComponentTickFunction*> TickableComponents;

	FTickablesTickFunction ComponentsTick;

	/* Cleanup after tick to avoid asserts when we get destroyed during iteration */
	TArray<FActorComponentTickFunction*> CleanupQueue;
};
