// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameModeBase.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AI/RogueAICharacter.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Player/RoguePlayerCharacter.h"
#include "Player/RoguePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameStateBase.h"
#include "RogueMonsterData.h"
#include "ActionRoguelike.h"
#include "RogueGameplayFunctionLibrary.h"
#include "ActionSystem/RogueActionComponent.h"
#include "SaveSystem/RogueSaveGameSubsystem.h"
#include "Development/RogueDeveloperSettings.h"
#include "Engine/AssetManager.h"
#include "Performance/RogueActorPoolingSubsystem.h"
#include "UI/RogueHUD.h"
#include "Windows/WindowsPlatformPerfCounters.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueGameModeBase)



ARogueGameModeBase::ARogueGameModeBase()
{
	PlayerStateClass = ARoguePlayerState::StaticClass();
	HUDClass = ARogueHUD::StaticClass();
}


void ARogueGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// (Save/Load logic moved into new SaveGameSubsystem)
	URogueSaveGameSubsystem* SG = GetGameInstance()->GetSubsystem<URogueSaveGameSubsystem>();

	// Optional slot name (Falls back to slot specified in SaveGameSettings class/INI otherwise)
	FString SelectedSaveSlot = UGameplayStatics::ParseOption(Options, "SaveGame");
	SG->LoadSaveGame(SelectedSaveSlot);
}


void ARogueGameModeBase::StartPlay()
{
	Super::StartPlay();

	AvailableSpawnCredit = InitialSpawnCredit;

	StartSpawningBots();
	
	// Make sure we have assigned at least one power-up class
	if (ensure(PowerupClasses.Num() > 0))
	{
		// Skip the Blueprint wrapper and use the direct C++ option which the Wrapper uses as well
		FEnvQueryRequest Request(PowerupSpawnQuery, this);
		Request.Execute(EEnvQueryRunMode::AllMatching, this, &ARogueGameModeBase::OnPowerupSpawnQueryCompleted);
	}
	
	// We run the prime logic after the BeginPlay call to avoid accidentally running that on stored/primed actors
	RequestPrimedActors();
}


void ARogueGameModeBase::RequestPrimedActors()
{
	URogueActorPoolingSubsystem* PoolingSystem = GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
	if (PoolingSystem->IsPoolingEnabled(this))
	{
		for (auto& Entry : ActorPoolClasses)
		{
			PoolingSystem->PrimeActorPool(Entry.Key, Entry.Value);
		}
	}
}


void ARogueGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Calling Before Super:: so we set variables before 'beginplayingstate' is called in PlayerController (which is where we instantiate UI)
	URogueSaveGameSubsystem* SG = GetGameInstance()->GetSubsystem<URogueSaveGameSubsystem>();
	SG->HandleStartingNewPlayer(NewPlayer);

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// Now we're ready to override spawn location
	// Alternatively we could override core spawn location to use store locations immediately (skipping the whole 'find player start' logic)
	SG->OverrideSpawnTransform(NewPlayer);
}


void ARogueGameModeBase::StartSpawningBots()
{
	// Continuous timer to spawn in more bots.
	// Actual amount of bots and whether it's allowed to spawn determined by spawn logic later in the chain...
	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &ARogueGameModeBase::SpawnBotTimerElapsed, SpawnTimerInterval, true);
}


void ARogueGameModeBase::SpawnBotTimerElapsed()
{
#if !UE_BUILD_SHIPPING
    // disabled as we now use big button in level for debugging, but in normal gameplay something like this is useful
    // does require some code update on how it handles this as 'override' currently not properly set up.
	if (GetDefault<URogueDeveloperSettings>()->bDisableSpawnBotsOverride)
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

	LogOnScreen(this, FString::Printf(TEXT("Available SpawnCredits: %f"), AvailableSpawnCredit));

	// Count alive bots before spawning
	int32 NrOfAliveBots = 0;
	// TActorRange simplifies the code compared to TActorIterator<T> (uses internally cached list of all ARogueAICharacter instances)
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

		LogOnScreen(this, FString::Printf(TEXT("Cooling down until: %f"), CooldownBotSpawnUntil), FColor::Red);
		return;
	}

	// Skip the Blueprint wrapper and use the direct C++ option which the Wrapper uses as well
	FEnvQueryRequest Request(SpawnBotQuery, this);

	FQueryFinishedSignature FinishedDelegate = FQueryFinishedSignature::CreateUObject(this, &ARogueGameModeBase::OnBotSpawnQueryCompleted, SelectedRow);
	
	Request.Execute(EEnvQueryRunMode::RandomBest5Pct, FinishedDelegate);
}


void ARogueGameModeBase::OnBotSpawnQueryCompleted(TSharedPtr<FEnvQueryResult> Result, FMonsterInfoRow* SelectedRow)
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
		FStreamableDelegate Delegate = FStreamableDelegate::CreateUObject(this, &ARogueGameModeBase::OnMonsterLoaded, MonsterId, Locations[0]);
		Manager.LoadPrimaryAsset(MonsterId, Bundles, Delegate);
	}
}


void ARogueGameModeBase::OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation)
{
	UAssetManager& Manager = UAssetManager::Get();

	URogueMonsterData* MonsterData = CastChecked<URogueMonsterData>(Manager.GetPrimaryAssetObject(LoadedId));
	
	AActor* NewBot = GetWorld()->SpawnActor<AActor>(MonsterData->MonsterClass, SpawnLocation, FRotator::ZeroRotator);
	// Spawn might fail if colliding with environment
	if (NewBot)
	{
		LogOnScreen(this, FString::Printf(TEXT("Spawned enemy: %s (%s)"), *GetNameSafe(NewBot), *GetNameSafe(MonsterData)));

		// Grant special actions, buffs etc.
		URogueActionComponent* ActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(NewBot);
		check(ActionComp);
		
		for (TSubclassOf<URogueAction> ActionClass : MonsterData->Actions)
		{
			ActionComp->AddAction(NewBot, ActionClass);
		}
	}
}


void ARogueGameModeBase::OnPowerupSpawnQueryCompleted(TSharedPtr<FEnvQueryResult> Result)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(OnPowerupSpawnQueryCompleted);
	
	FEnvQueryResult* QueryResult = Result.Get();
	if (!QueryResult->IsSuccessful())
	{
		UE_LOGFMT(LogGame, Warning, "Spawn bot EQS Query Failed!");
		return;
	}
	

	uint64 CyclesStart = FPlatformTime::Cycles64();

	// Retrieve all possible locations that passed the query
	TArray<FVector> Locations;
	QueryResult->GetAllAsLocations(Locations);

	// Keep used locations to easily check distance between points
	TArray<FVector> UsedLocations;

	int32 SpawnCounter = 0;
	// Break out if we reached the desired count or if we have no more potential positions remaining
	while (SpawnCounter < DesiredPowerupCount && Locations.Num() > 0)
	{
		// Pick a random location from remaining points.
		int32 RandomLocationIndex = FMath::RandRange(0, Locations.Num() - 1);

		FVector PickedLocation = Locations[RandomLocationIndex];
		// Remove to avoid picking again
		Locations.RemoveAtSwap(RandomLocationIndex);

		// Check minimum distance requirement
		bool bValidLocation = true;
		for (FVector OtherLocation : UsedLocations)
		{
			float DistanceTo = (PickedLocation - OtherLocation).Size();

			if (DistanceTo < RequiredPowerupDistance)
			{
				// Show skipped locations due to distance
				//DrawDebugSphere(GetWorld(), PickedLocation, 50.0f, 20, FColor::Red, false, 10.0f);

				// too close, skip to next attempt
				bValidLocation = false;
				break;
			}
		}

		// Failed the distance test
		if (!bValidLocation)
		{
			continue;
		}

		// Pick a random powerup-class
		int32 RandomClassIndex = FMath::RandRange(0, PowerupClasses.Num() - 1);
		TSubclassOf<AActor> RandomPowerupClass = PowerupClasses[RandomClassIndex];

		GetWorld()->SpawnActor<AActor>(RandomPowerupClass, PickedLocation, FRotator::ZeroRotator);

		// Keep for distance checks
		UsedLocations.Add(PickedLocation);
		SpawnCounter++;
	}

	uint64 CyclesEnd = FPlatformTime::Cycles64();

	UE_LOG(LogGame, Log, TEXT("OnPowerupSpawnQueryCompleted: %llu Cycles"), (CyclesEnd - CyclesStart));
}


void ARogueGameModeBase::RespawnPlayerElapsed(AController* Controller)
{
	Controller->UnPossess();
	RestartPlayer(Controller);
}


void ARogueGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
	UE_LOGFMT(LogGame, Log, "OnActorKilled: Victim: {victim}, Killer: {killer}", GetNameSafe(VictimActor), GetNameSafe(Killer));

	// Handle Player death
	ARoguePlayerCharacter* Player = Cast<ARoguePlayerCharacter>(VictimActor);
	if (Player)
	{
		// Auto-respawn
		if (bAutoRespawnPlayer)
		{
			FTimerHandle TimerHandle_RespawnDelay;
			FTimerDelegate Delegate;
			Delegate.BindUObject(this, &ThisClass::RespawnPlayerElapsed, Player->GetController());
 
			const float RespawnDelay = 2.0f;
			GetWorldTimerManager().SetTimer(TimerHandle_RespawnDelay, Delegate, RespawnDelay, false);
		}

		// Store time if it was better than previous record
		ARoguePlayerState* PS = Player->GetPlayerState<ARoguePlayerState>();
		if (PS)
		{
			PS->UpdatePersonalRecord(GetWorld()->TimeSeconds);
		}

		URogueSaveGameSubsystem* SG = GetGameInstance()->GetSubsystem<URogueSaveGameSubsystem>();
		// Immediately auto save on death
		SG->WriteSaveGame();
	}

	// Give Credits for kill
	APawn* KillerPawn = Cast<APawn>(Killer);
	// Don't credit kills of self
	if (KillerPawn && KillerPawn != VictimActor)
	{
		// Only Players will have a 'PlayerState' instance, bots have nullptr
		ARoguePlayerState* PS = KillerPawn->GetPlayerState<ARoguePlayerState>();
		if (PS) 
		{
			PS->AddCredits(CreditsPerKill);
		}
	}
}
