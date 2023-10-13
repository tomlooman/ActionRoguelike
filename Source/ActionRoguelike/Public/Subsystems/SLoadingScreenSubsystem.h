// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SLoadingScreenSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:

	void BeginLoadingScreen(const FString& MapName);

	void EndLoadingScreen(UWorld* InLoadedWorld);
};
