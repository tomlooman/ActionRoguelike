// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerState.h"
#include "SSaveGame.h"
#include "Net/UnrealNetwork.h"




void ASPlayerState::AddCredits(int32 Delta)
{
	// Avoid user-error of adding a negative amount
	if (!ensure(Delta >= 0.0f))
	{
		return;
	}

	Credits += Delta;

	OnCreditsChanged.Broadcast(this, Credits, Delta);
}


bool ASPlayerState::RemoveCredits(int32 Delta)
{
	// Avoid user-error of adding a subtracting negative amount
	if (!ensure(Delta >= 0.0f))
	{
		return false;
	}

	if (Credits < Delta)
	{
		// Not enough credits available
		return false;
	}

	Credits -= Delta;

	OnCreditsChanged.Broadcast(this, Credits, -Delta);

	return true;
}


void ASPlayerState::SavePlayerState_Implementation(USSaveGame* SaveObject)
{
	if (SaveObject)
	{
		SaveObject->Credits = Credits;
	}
}


void ASPlayerState::LoadPlayerState_Implementation(USSaveGame* SaveObject)
{
	if (SaveObject)
	{
		//Credits = SaveObject->Credits;
		// Makes sure we trigger credits changed event
		AddCredits(SaveObject->Credits);
	}
}


void ASPlayerState::OnRep_Credits(int32 OldCredits)
{
	OnCreditsChanged.Broadcast(this, Credits, Credits - OldCredits);
}


// void ASPlayerState::MulticastCredits_Implementation(float NewCredits, float Delta)
// {
// 	OnCreditsChanged.Broadcast(this, NewCredits, Delta);
// }


int32 ASPlayerState::GetCredits() const
{
	return Credits;
}


void ASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPlayerState, Credits);
}