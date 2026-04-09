// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueCoinPickupSubsystem.h"

#include "ActionRoguelike.h"
#include "EngineUtils.h"
#include "Components/AudioComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Core/RogueDeveloperSettings.h"
#include "Player/RoguePlayerCharacter.h"


void URogueCoinPickupSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UWorld* World = GetWorld();

	WorldISM = NewObject<UInstancedStaticMeshComponent>(World, NAME_None, RF_Transient);
	WorldISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WorldISM->RegisterComponentWithWorld(World);

	const URogueDeveloperSettings* DevSettings = GetDefault<URogueDeveloperSettings>();
	CoinPickupTriggerParamName = DevSettings->CoinPickupTriggerParameter;
	
	DevSettings->CoinPickupMesh.LoadAsync(
		FLoadSoftObjectPathAsyncDelegate::CreateUObject(this, &ThisClass::OnPickupMeshLoadComplete));

	WorldAudioComp = NewObject<UAudioComponent>(World, NAME_None, RF_Transient);
	WorldAudioComp->SetAutoActivate(false);
	WorldAudioComp->RegisterComponentWithWorld(World);

	DevSettings->CoinPickupSound.LoadAsync(
		FLoadSoftObjectPathAsyncDelegate::CreateUObject(this, &ThisClass::OnPickupSoundLoadComplete));
}

void URogueCoinPickupSubsystem::OnPickupMeshLoadComplete(const FSoftObjectPath& SoftObjectPath, UObject* LoadedObject)
{
	WorldISM->SetStaticMesh(Cast<UStaticMesh>(LoadedObject));
}

void URogueCoinPickupSubsystem::OnPickupSoundLoadComplete(const FSoftObjectPath& SoftObjectPath, UObject* LoadedObject)
{
	WorldAudioComp->SetSound(Cast<USoundBase>(LoadedObject));
}

void URogueCoinPickupSubsystem::PlayPickupSound()
{
	if (!WorldAudioComp->IsPlaying())
	{
		WorldAudioComp->Play();
	}

	WorldAudioComp->SetTriggerParameter(CoinPickupTriggerParamName);
}

void URogueCoinPickupSubsystem::AddCoinPickups(TArray<FVector> NewLocations, TArray<int32> NewAmounts)
{
	CoinLocations.Append(NewLocations);
	CoinAmounts.Append(NewAmounts);

	TArray<FTransform> MeshTransforms;
	for (int i = 0; i < NewLocations.Num(); ++i)
	{
		MeshTransforms.Add(FTransform(NewLocations[i] + FVector(0, 0, 50.0f)));
	}

	TArray<FPrimitiveInstanceId> NewMeshIDs = WorldISM->AddInstancesById(MeshTransforms, true,false);

	MeshIDs.Append(NewMeshIDs);
}

void URogueCoinPickupSubsystem::RemoveCoinPickup(int32 IndexToRemove)
{
	CoinLocations.RemoveAt(IndexToRemove);
	CoinAmounts.RemoveAt(IndexToRemove);

	WorldISM->RemoveInstanceById(MeshIDs[IndexToRemove]);
	MeshIDs.RemoveAt(IndexToRemove);
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

	if (TotalCoinsToGrant > 0)
	{
		PlayPickupSound();
	}

	// @todo: grant coins to player(s)
	UE_CLOG(TotalCoinsToGrant > 0, LogGame, Log, TEXT("Picked up Coin Amount=%d"), TotalCoinsToGrant);

	for (int i = 0; i < CoinLocations.Num(); ++i)
	{
		DrawDebugPoint(World, CoinLocations[i], 8.0f, FColor::White);
	}
}
