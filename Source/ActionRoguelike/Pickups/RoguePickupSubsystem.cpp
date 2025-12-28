// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupSubsystem.h"

#include "EngineUtils.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Player/RoguePlayerCharacter.h"



void URoguePickupSubsystem::AddNewCreditsPickup(FVector Origin, int32 CreditAmount)
{
	CreditPickupLocations.Add(Origin);
	CreditPickupAmount.Add(CreditAmount);
	//CreditsPickupDebugList.Add(false);

	UNiagaraComponent* NewComp = nullptr; /*= UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, CoinPickupFXTemplate, Origin, FRotator::ZeroRotator,
		FVector(1), true, true, ENCPoolMethod::ManualRelease);*/

	CreditFXList.Add(NewComp);
}

void URoguePickupSubsystem::RemoveCreditsPickup(int32 InIndex)
{
	CreditPickupLocations.RemoveAt(InIndex);
	CreditPickupAmount.RemoveAt(InIndex);

	// Manually pass back to pool
	//CreditFXList[InIndex]->ReleaseToPool();
	CreditFXList.RemoveAt(InIndex);
}

void URoguePickupSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(RoguePickupSubsystem::Tick)

		// Player Locations
		FVector PlayerLocation = FVector::ZeroVector;
		for (ARoguePlayerCharacter* PlayerPawn : TActorRange<ARoguePlayerCharacter>(World))
		{
			PlayerLocation = PlayerPawn->GetActorLocation();
		}

		const float PickupRadiusSqrd = PickupRadius * PickupRadius;

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

		for (int i = ProcessList.Num() - 1; i >= 0; --i)
		{
			RemoveCreditsPickup(ProcessList[i]);
			//CreditsPickupDebugList[ProcessList[i]] = true;
		}
	}

	// Debug Rendering
	for (int Index = 0; Index < CreditPickupLocations.Num(); ++Index)
	{
		DrawDebugBox(World, CreditPickupLocations[Index], FVector(5.0f), /*CreditsPickupDebugList[Index] ? FColor::Green :*/ FColor::Blue);
	}
}
