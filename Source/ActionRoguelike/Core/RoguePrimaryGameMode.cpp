// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePrimaryGameMode.h"

#include "ActionRoguelike.h"
#include "RogueGameInstance.h"
#include "RogueGameplayFunctionLibrary.h"
#include "RogueMonsterData.h"
#include "ActionSystem/RogueAction.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Development/RogueDeveloperLocalSettings.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "GameRules/RogueSpawnDirectors.h"


ARoguePrimaryGameMode::ARoguePrimaryGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	// 0.1sec is plenty for handling the frequency of credits, spawning, etc.
	PrimaryActorTick.TickInterval = 0.1f;
}


void ARoguePrimaryGameMode::StartPlay()
{
	Super::StartPlay();
	
	FRandomStream GlobalStream = FRandomStream(StartingSeed);

	// Init here, modify the director seeds before we init the stream
	for (FRogueDirectorData& Director : Directors)
	{
		float NewSeed = GlobalStream.FRandRange(0.0f, MAX_int32);
		Director.RandomStream_MonsterSelection = FRandomStream(NewSeed);
		
#if WITH_EDITORONLY_DATA
		// Validate we get consistent random numbers between runs
		UE_LOG(LogGame, Log, TEXT("Director '%s' seeded with %f"), *Director.EditorDisplayName, NewSeed);
#endif
	}
}


void ARoguePrimaryGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
#if !UE_BUILD_SHIPPING
	// disabled as we now use big button in level for debugging, but in normal gameplay something like this is useful
	if (DevelopmentOnly::GDisableSpawnBotsOverride)
	{
		return;
	}
#endif

	const float CurrGameTime = GetWorld()->TimeSeconds;

	for (FRogueDirectorData& Director : Directors)
	{
		if (Director.MonsterTable == nullptr)
		{
#if WITH_EDITORONLY_DATA
			// @todo: this will spam the log until fixed...should log once
			UE_LOG(LogGame, Warning, TEXT("Director %s is missing a valid DataTable."), *Director.EditorDisplayName);
#endif
			continue;
		}
		
		const float PlayerCountMultiplier = 1.0f;
		float NrOfAlivePlayers = 1; // @todo: grab from cached array, just like QueryContext_AlivePlayers	
		const float DifficultyMultiplier = 1.0f;
		const float DefaultCreditValue = 1.0f;
		
		// Award Credits
		float BaseCredits = Director.CreditsGainCurve.GetRichCurve()->Eval(CurrGameTime, DefaultCreditValue) * DeltaSeconds;
		Director.CurrentCredits += BaseCredits * DifficultyMultiplier * (PlayerCountMultiplier * NrOfAlivePlayers);
		
		// Check if we should "tick"
		if (Director.NextTickTime > CurrGameTime)
		{
			continue;	
		}
		
		// Try Spawn
		bool bSuccess = TrySpawnMonster(Director);
		
		// Setup next "tick"
		Director.NextTickTime = CurrGameTime + (bSuccess ? Director.TickInterval : Director.TimeBetweenWaves);
	}
}


bool ARoguePrimaryGameMode::TrySpawnMonster(FRogueDirectorData& DirectorData)
{
	URogueGameInstance* GI = Cast<URogueGameInstance>(GetGameInstance());
	int32 NrOfAliveBots = GI->AliveMonsters.Num();

	if (NrOfAliveBots >= NrMaxEnemies)
	{
		UE_LOGFMT(LogGame, Log, "At maximum bot capacity. Skipping bot spawn.");
		return false;
	}

	// Row to pass along with EQS delegate
	FMonsterInfoRow* SelectedRow = nullptr;

	TArray<FMonsterInfoRow*> Rows;
	DirectorData.MonsterTable->GetAllRows("", Rows);

	// Get total weight
	float TotalWeight = 0;
	for (FMonsterInfoRow* Entry : Rows)
	{
		TotalWeight += Entry->Weight;
	}

	// Random number within total random
	int32 RandomWeight = DirectorData.RandomStream_MonsterSelection.FRandRange(0.0f, TotalWeight);

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

	if (SelectedRow && SelectedRow->SpawnCost >= DirectorData.CurrentCredits)
	{
		// Too expensive to spawn, try again soon
		return false;
	}
	
	// Immediately apply cost.
	DirectorData.CurrentCredits -= SelectedRow->SpawnCost;
	
	// Find a spawn location
	FEnvQueryRequest Request(DirectorData.SpawnLocationQuery, this);
	Request.Execute(EEnvQueryRunMode::RandomBest5Pct, 
		FQueryFinishedSignature::CreateUObject(this, &ThisClass::SpawnQueryCompleted, SelectedRow));
	
	// Success
	return true;
}


void ARoguePrimaryGameMode::SpawnQueryCompleted(TSharedPtr<FEnvQueryResult> Result, FMonsterInfoRow* SelectedRow)
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
	
	if (Locations.IsValidIndex(0))
	{
		FPrimaryAssetId MonsterId = SelectedRow->MonsterId;

		TArray<FName> Bundles;
		FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ThisClass::OnMonsterLoaded, MonsterId, Locations[0]);
		UAssetManager::Get().LoadPrimaryAsset(MonsterId, Bundles, Delegate);
	}
}


void ARoguePrimaryGameMode::OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation)
{
	URogueMonsterData* MonsterData = CastChecked<URogueMonsterData>(UAssetManager::Get().GetPrimaryAssetObject(LoadedId));

	// Spawn might fail if colliding with environment
	if (AActor* NewBot = GetWorld()->SpawnActor<AActor>(MonsterData->MonsterClass, SpawnLocation, FRotator::ZeroRotator))
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

