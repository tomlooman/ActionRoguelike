// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// Define new "stat command" group shown in-game via "stat stanford"
DECLARE_STATS_GROUP(TEXT("STANFORD_Game"), STATGROUP_STANFORD, STATCAT_Advanced);

// Define category "LogGame"
ACTIONROGUELIKE_API DECLARE_LOG_CATEGORY_EXTERN(LogGame, Log, All);


static void LogOnScreen(UObject* WorldContext, FString Msg, FColor Color = FColor::White, float Duration = 5.0f)
{
	if (!ensure(WorldContext))
	{
		return;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!ensure(World))
	{
		return;
	}

	FString NetPrefix = World->IsNetMode(NM_Client) ? "[CLIENT] " : "[SERVER] ";
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, NetPrefix + Msg);
	}
}