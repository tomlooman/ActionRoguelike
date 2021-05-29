// Fill out your copyright notice in the Description page of Project Settings.


#include "SSaveGame.h"
#include "GameFramework/PlayerState.h"




FPlayerSaveData* USSaveGame::GetPlayerData(APlayerState* PlayerState)
{
	if (PlayerState == nullptr)
	{
		return nullptr;
	}

	// Will not give unique ID while PIE so we skip that step while testing in editor.
	// UObjects don't have access to UWorld, so we grab it via PlayerState instead
	if (PlayerState->GetWorld()->IsPlayInEditor())
	{
		UE_LOG(LogTemp, Log, TEXT("During PIE we cannot use PlayerID to retrieve Saved Player data. Using first entry in array if available."));

		if (SavedPlayers.IsValidIndex(0))
		{
			return &SavedPlayers[0];
		}

		// No saved player data available
		return nullptr;
	}

	int32 PlayerID = PlayerState->GetPlayerId();
	// Iterate the array and match by PlayerID (eg. unique ID provided by Steam)
	return SavedPlayers.FindByPredicate([&](const FPlayerSaveData& Data) { return Data.PlayerID == PlayerID; });
}
