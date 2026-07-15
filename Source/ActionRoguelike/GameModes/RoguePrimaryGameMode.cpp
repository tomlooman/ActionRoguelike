// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePrimaryGameMode.h"

#include "ActionRoguelike.h"
#include "RogueGameTypes.h"
#include "ActionSystem/RogueActionSystemComponent.h"
#include "AI/RogueAICharacter.h"
#include "AI/RogueMonsterData.h"
#include "Core/RogueGameInstance.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Kismet/GameplayStatics.h"


TAutoConsoleVariable<bool> CVarGameBotSpawningEnabled(
	TEXT("game.BotSpawningEnabled"),
	false,
	TEXT("Allows disabling of bot spawning for debugging purposes."),
	ECVF_Cheat);

TAutoConsoleVariable<int32> CVarGameBotLimit(
	TEXT("game.BotLimit"),
	5,
	TEXT("Define the maximum number of alive bots in the world."),
	ECVF_Default);


ARoguePrimaryGameMode::ARoguePrimaryGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
}


void ARoguePrimaryGameMode::StartPlay()
{
	Super::StartPlay();
	
	FRandomStream GlobalStream = FRandomStream(GlobalStartingSeed);

	for (FRogueDirectorData& Director : Directors)
	{
		int32 NewSeed = GlobalStream.RandRange(0, MAX_int32-1);
		Director.RandomStream_MonsterSelection = FRandomStream(NewSeed);
		
		UE_LOG(LogGameMode, Log, TEXT("Seed: %d for %s"), Director.RandomStream_MonsterSelection.GetInitialSeed(), *Director.DebugDisplayName);
	}
}

void ARoguePrimaryGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float TotalElapsedTime = GetWorld()->TimeSeconds;

	int32 KeyID = ONSCREENDEBUGKEY_SPAWNDIRECTOR;
	for (FRogueDirectorData& Director : Directors)
	{
		if (Director.MonsterSpawnTable == nullptr)
		{
			continue;
		}
		
		float CreditsPerSecond = Director.CreditGainCurve.GetRichCurve()->Eval(TotalElapsedTime);
		Director.CurrentCredits += CreditsPerSecond * DeltaSeconds;
		
		FString DebugMsg = FString::Printf(TEXT("%s\nCurrent Credits: %f\nNextTickTime: %f\n"), *Director.DebugDisplayName, Director.CurrentCredits, Director.NextTickTime);
		GEngine->AddOnScreenDebugMessage(KeyID, PrimaryActorTick.TickInterval, Director.DebugColor, DebugMsg);
		KeyID++;
		
		if (Director.NextTickTime > TotalElapsedTime)
		{
			continue;
		}
	
		bool bSuccess = TrySpawnMonster(Director);
		
		Director.NextTickTime = TotalElapsedTime + (bSuccess ? Director.TickInterval : Director.TimeBetweenWaves);
		
		//UE_LOG(LogGameMode, Log, TEXT("Total Credits: %f"), Director.CurrentCredits);
	}

}

bool ARoguePrimaryGameMode::TrySpawnMonster(FRogueDirectorData& Director)
{
	const int32 MaxBotLimit = CVarGameBotLimit.GetValueOnGameThread();
	URogueGameInstance* GI = GetGameInstance<URogueGameInstance>();
	if (GI->AliveMonsters.Num() >= MaxBotLimit)
	{
		UE_LOG(LogGameMode, Log, TEXT("Reached bot spawn limit of %d"), MaxBotLimit);
		return false;
	}
	
	TArray<FMonsterSpawnData*> AllRows;
	Director.MonsterSpawnTable->GetAllRows("SelectMonster", AllRows);
	
	//int32 SelectedIndex = Director.RandomStream_MonsterSelection.RandRange(0, AllRows.Num()-1);
	//FMonsterSpawnData* SelectedRow = AllRows[SelectedIndex];

	float TotalWeights = 0.0f;
	for (FMonsterSpawnData* Row : AllRows)
	{
		TotalWeights += Row->SpawnWeight;
	}
	
	float SelectedWeight = Director.RandomStream_MonsterSelection.FRandRange(0.0f, TotalWeights);
	
	// row 0 - 10 weight (10 total)
	// row 1 - 15 weight (25 total)
	// row 2 - 5 weight (30 total)
	// SelectedWeight (28) selects row 2, which ranges from 26-30 weight.

	FMonsterSpawnData* SelectedRow = nullptr;
	TotalWeights = 0.0f;
	for (FMonsterSpawnData* Row : AllRows)
	{
		TotalWeights += Row->SpawnWeight;
		if (SelectedWeight <= TotalWeights)
		{
			SelectedRow = Row;
			break;
		}
	}	
	
	if (Director.CurrentCredits < SelectedRow->SpawnCost)
	{
		UE_LOG(LogGameMode, Log, TEXT("Not enough credits to spawn monster %s"), *SelectedRow->MonsterData.GetAssetName());
		return false;
	}
	
	Director.CurrentCredits -= SelectedRow->SpawnCost;
	
	FQueryFinishedSignature CompleteDelegate = FQueryFinishedSignature::CreateUObject(this, &ThisClass::SpawnQueryCompleted, SelectedRow);
	
	FEnvQueryRequest Request(Director.SpawnLocationQuery, this);
	int32 QueryID = Request.Execute(EEnvQueryRunMode::SingleResult, CompleteDelegate);
	
	// EQS started successfully
	return QueryID != INDEX_NONE;
}

void ARoguePrimaryGameMode::SpawnQueryCompleted(TSharedPtr<FEnvQueryResult> QueryResult, FMonsterSpawnData* SelectedMonster)
{
	FVector SpawnLocation = QueryResult->GetItemAsLocation(0);
	
	SelectedMonster->MonsterData.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateUObject(this, &ThisClass::OnMonsterClassLoaded, SpawnLocation, SelectedMonster));
}

void ARoguePrimaryGameMode::OnMonsterClassLoaded(const FSoftObjectPath& LoadedObjectPath, UObject* LoadedObject, FVector SpawnLocation, FMonsterSpawnData* SelectedMonster)
{
	if (!CVarGameBotSpawningEnabled.GetValueOnGameThread())
	{
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	FTransform SpawnTM = FTransform(SpawnLocation);
	
	URogueMonsterData* MonsterData = SelectedMonster->MonsterData.Get();
	
	ARogueAICharacter* NewMonster = GetWorld()->SpawnActorDeferred<ARogueAICharacter>(MonsterData->MonsterClass, FTransform::Identity);
	
	NewMonster->SetMonsterData(MonsterData);
	
	// apply attribute overrides
	
	// Calls beginplay
	UGameplayStatics::FinishSpawningActor(NewMonster, SpawnTM);
	
	UE_VLOG_SPHERE(this, LogGameMode, Log, SpawnLocation, 32.0f, FColor::Blue, TEXT("MonsterType: %s\nCost:%.2f"), 
		*GetNameSafe(MonsterData->MonsterClass), SelectedMonster->SpawnCost);
	
	// add buffs/debuffs, etc.
	if (IsValid(NewMonster))
	{
		URogueActionSystemComponent* ActionComp = NewMonster->GetActionSystemComponent();

		for (TSubclassOf<URogueAction> ActionClass : MonsterData->Actions)
		{
			ActionComp->GrantAction(ActionClass);
		}
	}
}

