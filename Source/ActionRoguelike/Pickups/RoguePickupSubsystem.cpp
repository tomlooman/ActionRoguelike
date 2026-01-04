// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupSubsystem.h"

#include "EngineUtils.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Core/RogueDeveloperSettings.h"
#include "Core/RogueGameState.h"
#include "Player/RoguePlayerCharacter.h"



void URoguePickupSubsystem::AddCreditsPickup(FVector Origin, int32 CreditAmount)
{
	ENetMode NetMode = GetWorld()->GetNetMode();
	// Clients only react to data received from host
	check(GetWorld()->GetNetMode() != NM_Client);

	TRACE_CPUPROFILER_EVENT_SCOPE(RoguePickupSubsystem::AddCreditsPickup)
	
	CreditPickupLocations.Add(Origin);
	CreditPickupAmount.Add(CreditAmount);
	
	// Add locally (unbatched)
	const FPrimitiveInstanceId& Id = AddMeshInstance(Origin);
	MeshIDs.Add(Id);
	
	// Are we playing a networked game
	if (NetMode > NM_Standalone)
	{
		ARogueGameState* GS = GetWorld()->GetGameState<ARogueGameState>();

		FPickupLocationItem NewItem = FPickupLocationItem(Origin, Id);
		GS->CoinPickupData.Items.Add(NewItem);
		GS->CoinPickupData.MarkItemDirty(NewItem);
	}

}

void URoguePickupSubsystem::RemoveCreditsPickup(int32 InIndex)
{
	check(GetWorld()->GetNetMode() != NM_Client);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(RoguePickupSubsystem::RemoveCreditsPickup)
	
	CreditPickupLocations.RemoveAt(InIndex);
	CreditPickupAmount.RemoveAt(InIndex);

	ENetMode NetMode = GetWorld()->GetNetMode();
	// Playing any networked game, clients should not reach here in the first place
	if (NetMode > NM_Standalone)
	{
		ARogueGameState* GS = GetWorld()->GetGameState<ARogueGameState>();

		// Find match based on local ID again
		FPrimitiveInstanceId IdToFind = MeshIDs[InIndex];
		GS->CoinPickupData.Items.Remove(FPickupLocationItem(FVector::ZeroVector, IdToFind));
		GS->CoinPickupData.MarkArrayDirty();
	}

	WorldISM->RemoveInstanceById(MeshIDs[InIndex]);
	MeshIDs.RemoveAt(InIndex);
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


void URoguePickupSubsystem::CreateWorldISM()
{
	UWorld* World = GetWorld();

	// Temp sync loading of the mesh, can hitch
	UStaticMesh* Mesh = GetDefault<URogueDeveloperSettings>()->PickupCoinMesh.LoadSynchronous();
		
	WorldISM = NewObject<UInstancedStaticMeshComponent>(World, NAME_None, RF_Transient);
	WorldISM->SetStaticMesh(Mesh);
	WorldISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WorldISM->RegisterComponentWithWorld(World);
}

void URoguePickupSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(RoguePickupSubsystem::Tick)

		// Performance Note: This processing is laid out to move around and touch as few things per iteration
		// Therefor we first process all the possible coin pickups and count the total credits before we
		// award any of the players which may end up triggering a bunch of other delegates and pulling classes/data into memory
		
		// Note multiplayer: Even clients may run the logic locally to trigger cosmetic pickup events
		
		TArray<FVector> Players;
		TArray<ARoguePlayerCharacter*> PlayerPawns;
		TArray<int32> TotalCreditsPerPlayer;
		
		for (ARoguePlayerCharacter* PlayerPawn : TActorRange<ARoguePlayerCharacter>(World))
		{
			Players.Add(PlayerPawn->GetActorLocation());
		}

		// @todo: make this a player configured stat or attribute
		const float PickupRadius = 200.f;
		const float PickupRadiusSqrd = PickupRadius * PickupRadius;

		// Find pickups and track credits to grant
		for (FVector& PlayerLocation : Players)
		{
			// Track all pickups that need to be picked up.
			TArray<int32> ProcessList;

			for (int Index = 0; Index < CreditPickupLocations.Num(); ++Index)
			{
				float DistSqrd = FVector::DistSquared(CreditPickupLocations[Index], PlayerLocation);
				if (DistSqrd < PickupRadiusSqrd)
				{
					// Bookkeep all pickups that need processing for later
					ProcessList.Add(Index);
				}
			}

			int32 TotalCredits = 0;
			for (int i = ProcessList.Num() - 1; i >= 0; --i)
			{
				TotalCredits += CreditPickupAmount[ProcessList[i]];
				
				RemoveCreditsPickup(ProcessList[i]);
				//CreditsPickupDebugList[ProcessList[i]] = true;
			}

			TotalCreditsPerPlayer.Add(TotalCredits);
		}

		// Award each player
		for (int i = 0; i < PlayerPawns.Num(); ++i)
		{
			FAttributeModification Mod = FAttributeModification(SharedGameplayTags::Attribute_Credits, TotalCreditsPerPlayer[i]);

			PlayerPawns[i]->GetActionComponent()->ApplyAttributeChange(Mod);
		}
	}

	// Debug Rendering
	//for (int Index = 0; Index < CreditPickupLocations.Num(); ++Index)
	{
		//DrawDebugBox(World, CreditPickupLocations[Index], FVector(5.0f), /*CreditsPickupDebugList[Index] ? FColor::Green :*/ FColor::Blue);
	}
}
