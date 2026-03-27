// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueCoinPickupSubsystem.h"

#include "ActionRoguelike.h"
#include "EngineUtils.h"
#include "Player/RoguePlayerCharacter.h"

void URogueCoinPickupSubsystem::AddCoinPickups(TArray<FVector> NewLocations, TArray<int32> NewAmounts)
{
	CoinLocations.Append(NewLocations);
	CoinAmounts.Append(NewAmounts);
}

void URogueCoinPickupSubsystem::RemoveCoinPickup(int32 IndexToRemove)
{
	CoinLocations.RemoveAt(IndexToRemove);
	CoinAmounts.RemoveAt(IndexToRemove);
}

void URogueCoinPickupSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();

	FVector PlayerLocation = FVector::ZeroVector;
	for (ARoguePlayerCharacter* PlayerCharacter : TActorRange<ARoguePlayerCharacter>(World))
	{
		PlayerLocation = PlayerCharacter->GetActorLocation();
	}

	const float PickupRadius = 200.0f;

	TArray<int32> ProcessList;
	
	for (int i = 0; i < CoinLocations.Num(); ++i)
	{
		float Dist = FVector::Dist(PlayerLocation, CoinLocations[i]);
		if (Dist < PickupRadius)
		{
			ProcessList.Add(i);
		}
	}

	int32 TotalCoinsToGrant = 0;
	for (int i = ProcessList.Num()-1; i >= 0; --i)
	{
		int32 CoinIndex = ProcessList[i];
		
		TotalCoinsToGrant += CoinAmounts[CoinIndex];

		RemoveCoinPickup(CoinIndex);
	}

	// @todo: grant coins to player(s)
	UE_CLOG(TotalCoinsToGrant > 0, LogGame, Log, TEXT("Picked up Coin Amount=%d"), TotalCoinsToGrant);

	for (int i = 0; i < CoinLocations.Num(); ++i)
	{
		DrawDebugPoint(World, CoinLocations[i], 8.0f, FColor::White);
	}
}
