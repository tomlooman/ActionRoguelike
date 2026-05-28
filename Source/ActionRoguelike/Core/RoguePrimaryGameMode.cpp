// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePrimaryGameMode.h"

#include "ActionRoguelike.h"
#include "EngineUtils.h"
#include "RogueGameplayFunctionLibrary.h"
#include "RogueMonsterData.h"
#include "ActionSystem/RogueAction.h"
#include "ActionSystem/RogueActionComponent.h"
#include "AI/RogueAICharacter.h"
#include "Development/RogueDeveloperLocalSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "EnvironmentQuery/EnvQueryManager.h"

void ARoguePrimaryGameMode::StartPlay()
{
	Super::StartPlay();
	
	AvailableSpawnCredit = InitialSpawnCredit;

	StartSpawningBots();
	
}

void ARoguePrimaryGameMode::StartSpawningBots()
{
	// Continuous timer to spawn in more bots.
	// Actual amount of bots and whether it's allowed to spawn determined by spawn logic later in the chain...
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &ThisClass::SpawnBotTimerElapsed, SpawnTimerInterval, true);
}


void ARoguePrimaryGameMode::SpawnBotTimerElapsed()
{
#if !UE_BUILD_SHIPPING
    // disabled as we now use big button in level for debugging, but in normal gameplay something like this is useful
	if (DevelopmentOnly::GDisableSpawnBotsOverride)
	{
		return;
	}
#endif

	// Give points to spend
	if (SpawnCreditCurve)
	{
		AvailableSpawnCredit += SpawnCreditCurve->GetFloatValue(GetWorld()->TimeSeconds);
	}

	if (CooldownBotSpawnUntil > GetWorld()->TimeSeconds)
	{
		// Still cooling down
		return;
	}
	
	// Count alive bots before spawning
	int32 NrOfAliveBots = 0;
	for (ARogueAICharacter* Bot : TActorRange<ARogueAICharacter>(GetWorld()))
	{
		if (URogueGameplayFunctionLibrary::IsAlive(Bot))
		{
			NrOfAliveBots++;
		}
	}

	UE_LOGFMT(LogGame, Log, "Found {number} alive bots.", NrOfAliveBots);

	const float MaxBotCount = 10.0f;
	if (NrOfAliveBots >= MaxBotCount)
	{
		UE_LOGFMT(LogGame, Log, "At maximum bot capacity. Skipping bot spawn.");
		return;
	}

	// Row to pass along with EQS delegate
	FMonsterInfoRow* SelectedRow = nullptr;

	// @todo: warn about no monsterrow much earlier in the game and don't even bother arriving here if not set.
	// Use either DataValidation, asserts, or combination to prevent this from crashing here.
	//if (MonsterTable)
	
	TArray<FMonsterInfoRow*> Rows;
	MonsterTable->GetAllRows("", Rows);

	// Get total weight
	float TotalWeight = 0;
	for (FMonsterInfoRow* Entry : Rows)
	{
		TotalWeight += Entry->Weight;
	}

	// Random number within total random
	int32 RandomWeight = FMath::RandRange(0.0f, TotalWeight);

	//Reset
	TotalWeight = 0;

	// Get monster based on random weight
	for (FMonsterInfoRow* Entry : Rows)
	{
		TotalWeight += Entry->Weight;

		if (RandomWeight <= TotalWeight)
		{
			SelectedRow = Entry;
			break;
		}
	}

	if (SelectedRow && SelectedRow->SpawnCost >= AvailableSpawnCredit)
	{
		// Too expensive to spawn, try again soon
		CooldownBotSpawnUntil = GetWorld()->TimeSeconds + CooldownTimeBetweenFailures;
		return;
	}

	// Skip the Blueprint wrapper and use the direct C++ option which the Wrapper uses as well
	FEnvQueryRequest Request(SpawnBotQuery, this);

	FQueryFinishedSignature FinishedDelegate = FQueryFinishedSignature::CreateUObject(this, &ThisClass::OnBotSpawnQueryCompleted, SelectedRow);
	
	Request.Execute(EEnvQueryRunMode::RandomBest5Pct, FinishedDelegate);
}


void ARoguePrimaryGameMode::OnBotSpawnQueryCompleted(TSharedPtr<FEnvQueryResult> Result, FMonsterInfoRow* SelectedRow)
{
	FEnvQueryResult* QueryResult = Result.Get();
	if (!QueryResult->IsSuccessful())
	{
		UE_LOGFMT(LogGame, Warning, "Spawn bot EQS Query Failed!");
		return;
	}

	// Retrieve all possible locations that passed the query
	TArray<FVector> Locations;
	QueryResult->GetAllAsLocations(Locations);

	if (Locations.IsValidIndex(0) && MonsterTable)
	{
		UAssetManager& Manager = UAssetManager::Get();
		
		// Apply spawn cost
		AvailableSpawnCredit -= SelectedRow->SpawnCost;

		FPrimaryAssetId MonsterId = SelectedRow->MonsterId;

		TArray<FName> Bundles;
		FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnMonsterLoaded, MonsterId, Locations[0]);
		Manager.LoadPrimaryAsset(MonsterId, Bundles, Delegate);
	}
}


void ARoguePrimaryGameMode::OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation)
{
	UAssetManager& Manager = UAssetManager::Get();

	URogueMonsterData* MonsterData = CastChecked<URogueMonsterData>(Manager.GetPrimaryAssetObject(LoadedId));
	
	AActor* NewBot = GetWorld()->SpawnActor<AActor>(MonsterData->MonsterClass, SpawnLocation, FRotator::ZeroRotator);
	// Spawn might fail if colliding with environment
	if (NewBot)
	{
		// Grant special actions, buffs etc.
		URogueActionComponent* ActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(NewBot);
		check(ActionComp);
		
		for (TSubclassOf<URogueAction> ActionClass : MonsterData->Actions)
		{
			ActionComp->AddAction(NewBot, ActionClass);
		}
	}
}

