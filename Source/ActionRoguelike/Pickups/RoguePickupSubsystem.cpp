// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupSubsystem.h"

#include "EngineUtils.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Components/AudioComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Core/RogueDeveloperSettings.h"
#include "Core/RogueGameState.h"
#include "Player/RoguePlayerCharacter.h"
#include "ActionRoguelike.h"



void URoguePickupSubsystem::AddCoinsPickup(const TArray<FVector>& Locations, const TArray<int32>& CoinAmount)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(RoguePickupSubsystem::AddCoinsPickup)
	
	ENetMode NetMode = GetWorld()->GetNetMode();
	// Clients only react to data received from host
	check(GetWorld()->GetNetMode() != NM_Client);

	CoinPickupLocations.Append(Locations);
	CoinCreditRewards.Append(CoinAmount);

	// Convert to transforms for ISM
	TArray<FTransform> Transforms;
	Transforms.Reserve(Locations.Num());
	for (int i = 0; i < Locations.Num(); ++i)
	{
		Transforms.Add(FTransform(Locations[i]));
	}
	
	TArray<FPrimitiveInstanceId> NewMeshIDs = AddMeshInstances(Transforms);
	CoinMeshIDs.Append(NewMeshIDs);
	
	// Are we playing a networked game
	if (NetMode > NM_Standalone)
	{
		ARogueGameState* GS = GetWorld()->GetGameState<ARogueGameState>();

		// Grab Locations & Mesh IDs for replication
		// Note: Unclear if we can Append() and mark the items dirty, instead we just add one by one
		for (int i = 0; i < NewMeshIDs.Num(); ++i)
		{
			// Must mark dirty by reference later, so construct item directly in array rather than locally
			FPickupLocationItem& NewItem = GS->CoinPickupData.Items.Emplace_GetRef(FPickupLocationItem(Locations[i], NewMeshIDs[i]));
			GS->CoinPickupData.MarkItemDirty(NewItem);
		}
	}
}


void URoguePickupSubsystem::RemoveCoinsPickup(int32 InIndex)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(RoguePickupSubsystem::RemoveCoinsPickup)
	
	ENetMode NetMode = GetWorld()->GetNetMode();
	check(NetMode != NM_Client);
	
	CoinPickupLocations.RemoveAtSwap(InIndex, EAllowShrinking::No);
	CoinCreditRewards.RemoveAtSwap(InIndex, EAllowShrinking::No);

	// Playing any networked game, clients should not reach here in the first place
	if (NetMode > NM_Standalone)
	{
		ARogueGameState* GS = GetWorld()->GetGameState<ARogueGameState>();

		// Find match based on local ID again
		FPrimitiveInstanceId IdToFind = CoinMeshIDs[InIndex];
		GS->CoinPickupData.Items.RemoveSingleSwap(FPickupLocationItem(FVector::ZeroVector, IdToFind));
		GS->CoinPickupData.MarkArrayDirty();
	}

	WorldISM->RemoveInstanceById(CoinMeshIDs[InIndex]);
	CoinMeshIDs.RemoveAtSwap(InIndex, EAllowShrinking::No);
}

FPrimitiveInstanceId URoguePickupSubsystem::AddMeshInstance(FVector InLocation)
{
	// Lazy init
	if (!IsValid(WorldISM))
	{
		CreateWorldISM();
	}
	
	return WorldISM->AddInstanceById(FTransform(InLocation), true);
}

TArray<FPrimitiveInstanceId> URoguePickupSubsystem::AddMeshInstances(const TArray<FTransform>& InAdded)
{
	// Lazy init
	if (!IsValid(WorldISM))
	{
		CreateWorldISM();
	}

	// Batch-add
	return WorldISM->AddInstancesById(InAdded, true, false);
}

void URoguePickupSubsystem::RemoveMeshInstances(const TArray<FPrimitiveInstanceId>& IdsToRemove)
{
	check(WorldISM);
	WorldISM->RemoveInstancesById(IdsToRemove, false);
}


void URoguePickupSubsystem::CreateWorldISM()
{
	UWorld* World = GetWorld();

	// Temp sync loading of the mesh, can hitch
	UStaticMesh* Mesh = GetDefault<URogueDeveloperSettings>()->PickupCoinMesh.LoadSynchronous();
		
	WorldISM = NewObject<UInstancedStaticMeshComponent>(World, NAME_None, RF_Transient);
	WorldISM->SetStaticMesh(Mesh);
	WorldISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WorldISM->SetAffectDistanceFieldLighting(false);
	WorldISM->RegisterComponentWithWorld(World);
}


void URoguePickupSubsystem::PlayLocalCoinPickupSound()
{
	// The sound may still be loading when the first pickup arrives.
	if (!IsValid(CoinPickupAudioComp) || !CoinPickupAudioComp->GetSound())
	{
		return;
	}

	if (!CoinPickupAudioComp->IsPlaying())
	{
		CoinPickupAudioComp->Play();
	}

	// by repeatedly triggering this event we play a sequence of higher pitched pickups
	// The metasound handles "resetting" the pitch of the pickup sequence automatically
	CoinPickupAudioComp->SetTriggerParameter("CoinPickedUp");
}

void URoguePickupSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(RoguePickupSubsystem::Tick)

		// Performance Note: This processing is laid out to move around and touch as few things per iteration
		// Therefor we first process all the possible coin pickups and count the total Coins before we
		// award any of the players which may end up triggering a bunch of other delegates and pulling classes/data into memory

		TArray<FVector> Players;
		TArray<ARoguePlayerCharacter*> PlayerPawns;
		TArray<int32> TotalCoinsPerPlayer;
		
		for (ARoguePlayerCharacter* PlayerPawn : TActorRange<ARoguePlayerCharacter>(World))
		{
			if (!PlayerPawn->IsAlive())
			{
				continue;
			}

			Players.Add(PlayerPawn->GetActorLocation());
			PlayerPawns.Add(PlayerPawn);
		}

		// @todo: make this a player configured stat or attribute
		const float PickupRadius = 200.f;
		const float PickupRadiusSqrd = PickupRadius * PickupRadius;

		// Find pickups and track Coins to grant
		for (FVector& PlayerLocation : Players)
		{
			// Track all pickups that need to be picked up.
			TArray<int32> ProcessList;
			
#if USE_MULTITHREADED_COIN_PICKUPS
			// Multiple-producer, single consumer queue
			TMpscQueue<int32> ProcessQueue;
			
			// Min batch size is essential for getting any benefit out of doing this async as the per-iteration
			// cost for this function is so small (just a distance check)
			const int32 MinBatchSize = 1000;
			ParallelFor(TEXT("ParallelCoinTick"), CoinPickupLocations.Num(), MinBatchSize, [&](int32 Index)
			{
				// relatively high overhead vs. just distance checking, so it will skew the numbers
				//TRACE_CPUPROFILER_EVENT_SCOPE(CoinTick::DistanceCheck)
				
				float DistSqrd = FVector::DistSquared(CoinPickupLocations[Index], PlayerLocation);
				if (DistSqrd < PickupRadiusSqrd)
				{
					// Pickups need processing later to avoid messing with cpu cache					
					ProcessQueue.Enqueue(Index);
				}
			});
			
			int32 ProcessIndex; 
			while (ProcessQueue.Dequeue(ProcessIndex))
			{
				ProcessList.Add(ProcessIndex);
			}
			
			// Because of the multithreaded loop (and a queue rather than array) we need to first sort to use an inverse-for loop later
			Algo::Sort(ProcessList);
			
#else
			
			for (int Index = 0; Index < CoinPickupLocations.Num(); ++Index)
			{
				float DistSqrd = FVector::DistSquared(CoinPickupLocations[Index], PlayerLocation);
				if (DistSqrd < PickupRadiusSqrd)
				{
					// Bookkeep all pickups that need processing for later
					ProcessList.Add(Index);
				}
			}
#endif
			
			int32 TotalCoins = 0;
			for (int i = ProcessList.Num() - 1; i >= 0; --i)
			{
				TotalCoins += CoinCreditRewards[ProcessList[i]];
				
				RemoveCoinsPickup(ProcessList[i]);
			}
			
			TotalCoinsPerPlayer.Add(TotalCoins);
		}

		// Award each player
		for (int i = 0; i < PlayerPawns.Num(); ++i)
		{
			ARoguePlayerCharacter* MyPawn = PlayerPawns[i];
			int32 AwardAmount = TotalCoinsPerPlayer[i];
			if (AwardAmount == 0)
			{
				continue;
			}
			
			MyPawn->GetActionComponent()->ApplyAttributeChange(SharedGameplayTags::Attribute_Credits, AwardAmount, MyPawn, EAttributeModifyType::AddBase);
			MyPawn->ClientPlayCoinPickupSound();
		}
	}

	// Debug Rendering
	//for (int Index = 0; Index < CoinPickupLocations.Num(); ++Index)
	{
		//DrawDebugBox(World, CoinPickupLocations[Index], FVector(5.0f), FColor::Blue);
	}
}

bool URoguePickupSubsystem::IsTickable() const
{
	// Run everywhere except clients. Only standalone/host will check for "overlaps" during tick
	return GetWorld()->GetNetMode() < NM_Client;
}

void URoguePickupSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();

	CoinPickupAudioComp = NewObject<UAudioComponent>(World, NAME_None, RF_Transient);
	CoinPickupAudioComp->bAutoActivate = false;
	// Only the collecting player hears this feedback, so it stays 2D.
	CoinPickupAudioComp->bAllowSpatialization = false;
	CoinPickupAudioComp->RegisterComponentWithWorld(World);

	// Async load the sound
	FLoadSoftObjectPathAsyncDelegate Delegate;
	Delegate.BindUObject(this, &ThisClass::OnSoundAssetLoadComplete);
	GetDefault<URogueDeveloperSettings>()->PickupCoinSound.LoadAsync(Delegate);
}

void URoguePickupSubsystem::OnSoundAssetLoadComplete(const FSoftObjectPath& SoftObjectPath, UObject* LoadedObject)
{
	CoinPickupAudioComp->SetSound(Cast<USoundBase>(LoadedObject));
}
