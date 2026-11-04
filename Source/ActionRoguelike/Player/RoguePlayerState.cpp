// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/RoguePlayerState.h"

#include "ActionRoguelike.h"
#include "SaveSystem/RogueSaveGame.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePlayerState)


bool ARoguePlayerState::UpdatePersonalRecord(float NewTime)
{
	// Higher time is better
	if (NewTime > PersonalRecordTime)
	{
		float OldRecord = PersonalRecordTime;

		PersonalRecordTime = NewTime;

		OnRecordTimeChanged.Broadcast(this, PersonalRecordTime, OldRecord);

		return true;
	}

	return false;
}


void ARoguePlayerState::SavePlayerState_Implementation(URogueSaveGame* SaveObject)
{
	if (SaveObject)
	{
		// Gather all relevant data for player
		FPlayerSaveData SaveData;
		SaveData.PersonalRecordTime = PersonalRecordTime;
		// Stored as FString for simplicity (original Steam ID is uint64)
		SaveData.PlayerID = GetUniqueId().ToString();

		// May not be alive while we save
		if (APawn* MyPawn = GetPawn())
		{
			SaveData.Location = MyPawn->GetActorLocation();
			SaveData.Rotation = MyPawn->GetActorRotation();
			SaveData.bResumeAtTransform = true;
		}
		
		SaveObject->SavedPlayers.Add(SaveData);
	}
}


void ARoguePlayerState::LoadPlayerState_Implementation(URogueSaveGame* SaveObject)
{
	if (SaveObject)
	{
		FPlayerSaveData* FoundData = SaveObject->GetPlayerData(this);
		if (FoundData)
		{
			PersonalRecordTime = FoundData->PersonalRecordTime;
		}
		else
		{
			UE_LOGFMT(LogGame, Warning, "Could not find SaveGame data for player id: {playerid}.", GetPlayerId());
		}
	}
}