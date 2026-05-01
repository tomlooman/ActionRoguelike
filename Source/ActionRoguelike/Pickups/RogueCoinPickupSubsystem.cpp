// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueCoinPickupSubsystem.h"

#include "ActionRoguelike.h"
#include "EngineUtils.h"
#include "Components/AudioComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Core/RogueDeveloperSettings.h"
#include "Player/RoguePlayerCharacter.h"
#include "ProfilingDebugging/CountersTrace.h"


TRACE_DECLARE_INT_COUNTER(CoinInstanceCount, TEXT("Coins in World"));


void URogueCoinPickupSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UWorld* World = GetWorld();

	WorldISM = NewObject<UInstancedStaticMeshComponent>(World, NAME_None, RF_Transient);
	WorldISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WorldISM->RegisterComponentWithWorld(World);
	
	TRACE_COUNTER_SET(CoinInstanceCount, 0);

	const URogueDeveloperSettings* DevSettings = GetDefault<URogueDeveloperSettings>();
	CoinPickupTriggerParamName = DevSettings->CoinPickupTriggerParameter;
	
	DevSettings->CoinPickupMesh.LoadAsync(
		FLoadSoftObjectPathAsyncDelegate::CreateUObject(this, &ThisClass::OnPickupMeshLoadComplete));

	WorldAudioComp = NewObject<UAudioComponent>(World, NAME_None, RF_Transient);
	WorldAudioComp->SetAutoActivate(false);
	WorldAudioComp->bAllowSpatialization = false;
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
	TRACE_CPUPROFILER_EVENT_SCOPE(CoinPickupSubsystem::AddCoinPickups);
	
	
	CoinLocations.Append(NewLocations);
	CoinAmounts.Append(NewAmounts);

	TArray<FTransform> MeshTransforms;
	for (int i = 0; i < NewLocations.Num(); ++i)
	{
		MeshTransforms.Add(FTransform(NewLocations[i] + FVector(0, 0, 50.0f)));
	}

	TArray<FPrimitiveInstanceId> NewMeshIDs = WorldISM->AddInstancesById(MeshTransforms, true,false);

	MeshIDs.Append(NewMeshIDs);
	
	TRACE_COUNTER_SET(CoinInstanceCount, CoinLocations.Num());
}

void URogueCoinPickupSubsystem::RemoveCoinPickup(int32 IndexToRemove)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(CoinPickupSubsystem::RemoveCoinPickup);
	
	CoinLocations.RemoveAt(IndexToRemove);
	CoinAmounts.RemoveAt(IndexToRemove);

	WorldISM->RemoveInstanceById(MeshIDs[IndexToRemove]);
	MeshIDs.RemoveAt(IndexToRemove);
	
	TRACE_COUNTER_SET(CoinInstanceCount, CoinLocations.Num());
}

void URogueCoinPickupSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(CoinPickupSubsystem::Tick);
	
	UWorld* World = GetWorld();

	FVector PlayerLocation = FVector::ZeroVector;
	for (ARoguePlayerCharacter* PlayerCharacter : TActorRange<ARoguePlayerCharacter>(World))
	{
		PlayerLocation = PlayerCharacter->GetActorLocation();
	}

	const float PickupRadius = 200.0f;

	TArray<int32> ProcessList;
	
	// Distance Check
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(CoinPickupSubsystem::Tick::DistanceCheck);
		
		for (int i = 0; i < CoinLocations.Num(); ++i)
        {
        	float Dist = FVector::Dist(PlayerLocation, CoinLocations[i]);
        	if (Dist < PickupRadius)
        	{
        		ProcessList.Add(i);
        	}
        }
	}
	
	int32 TotalCoinsToGrant = 0;
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(CoinPickupSubsystem::Tick::HandlePickups);

		for (int i = ProcessList.Num()-1; i >= 0; --i)
		{
			int32 CoinIndex = ProcessList[i];
		
			TotalCoinsToGrant += CoinAmounts[CoinIndex];

			RemoveCoinPickup(CoinIndex);
		}
	}

	if (TotalCoinsToGrant > 0)
	{
		PlayPickupSound();
	}

#if 0
	// @todo: grant coins to player(s)
	UE_CLOG(TotalCoinsToGrant > 0, LogGame, Log, TEXT("Picked up Coin Amount=%d"), TotalCoinsToGrant);

	for (int i = 0; i < CoinLocations.Num(); ++i)
	{
		DrawDebugPoint(World, CoinLocations[i], 8.0f, FColor::White);
	}
#endif
}
