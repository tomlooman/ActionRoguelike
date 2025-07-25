// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// Added here to more easily include whenever we also use LogGame (structuredlog is new in 5.2)
#include "Logging/StructuredLog.h"

// Define category "LogGame"
ACTIONROGUELIKE_API DECLARE_LOG_CATEGORY_EXTERN(LogGame, Log, All);

// Define alias to easily keep track of custom channels in C++ (must match what is specified in Project Settings > Collision 
#define COLLISION_PROJECTILE ECC_GameTraceChannel1
#define TRACE_INTERACT ECC_GameTraceChannel2

// Perception System simple Team Numbers
#define TEAM_ID_BOTS 1
#define TEAM_ID_PLAYERS 2

namespace Animation
{
	static FName NAME_Foot_Plant_L = FName(TEXT("l_foot_plant"));
	static FName NAME_Foot_Plant_R = FName(TEXT("r_foot_plant"));
}

namespace Collision
{
	static FName Ragdoll_ProfileName = FName(TEXT("Ragdoll"));
	static FName Projectile_ProfileName = FName(TEXT("Projectile"));
	static FName Powerup_ProfileName = FName(TEXT("Powerup"));
}

namespace MeshSockets
{
	static FName RightHandMuzzle = FName(TEXT("Muzzle_01"));
	static FName LeftHandMuzzle = FName(TEXT("Muzzle_02"));
}


static void LogOnScreen(const UObject* WorldContext, const FString& Msg, FColor Color = FColor::White, float Duration = 5.0f)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);

	// Net Prefix is helpful during PIE
	FString FullMessage = World->IsNetMode(NM_Client) ? "[CLIENT] " : "[SERVER] " + Msg;
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, FullMessage);
	}

	UE_LOG(LogGame, Log, TEXT("%s"), *FullMessage);
}