// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePrimaryGameMode.h"

#include "ActionRoguelike.h"
#include "RogueGameTypes.h"
#include "AI/RogueAICharacter.h"
#include "Core/RogueGameInstance.h"
#include "EnvironmentQuery/EnvQueryManager.h"


ARoguePrimaryGameMode::ARoguePrimaryGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
}

void ARoguePrimaryGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	float TotalElapsedTime = GetWorld()->TimeSeconds;
	
	const int32 MaxBotLimit = 5;
	URogueGameInstance* GI = GetGameInstance<URogueGameInstance>();
	if (GI->AliveMonsters.Num() >= MaxBotLimit)
	{
		UE_LOG(LogGameMode, Log, TEXT("Reached bot spawn limit of %d"), MaxBotLimit);
		return;
	}	
	
	int32 KeyID = ONSCREENDEBUGKEY_SPAWNDIRECTOR;
	for (FRogueDirectorData& Director : Directors)
	{
		if (Director.MonsterSpawnTable == nullptr)
		{
			return;
		}
		
		float CreditsPerSecond = Director.CreditGainCurve.GetRichCurve()->Eval(TotalElapsedTime);
		Director.CurrentCredits += CreditsPerSecond * DeltaSeconds;
		
		FString DebugMsg = FString::Printf(TEXT("Current Credits: %f\nNextTickTime: %f"), Director.CurrentCredits, Director.NextTickTime);
		GEngine->AddOnScreenDebugMessage(KeyID, PrimaryActorTick.TickInterval, FColor::Blue, DebugMsg);
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
	TArray<FMonsterSpawnData*> AllRows;
	Director.MonsterSpawnTable->GetAllRows("SelectMonster", AllRows);
	
	int32 SelectedIndex = FMath::RandRange(0, AllRows.Num()-1);
	FMonsterSpawnData* SelectedRow = AllRows[SelectedIndex];
	
	if (Director.CurrentCredits < SelectedRow->SpawnCost)
	{
		UE_LOG(LogGameMode, Log, TEXT("Not enough credits to spawn monster %s"), *SelectedRow->MonsterClass.GetAssetName());
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
	
	SelectedMonster->MonsterClass.LoadAsync(FLoadSoftObjectPathAsyncDelegate::CreateUObject(this, &ThisClass::OnMonsterClassLoaded, SpawnLocation, SelectedMonster));
}

void ARoguePrimaryGameMode::OnMonsterClassLoaded(const FSoftObjectPath& LoadedObjectPath, UObject* LoadedObject, FVector SpawnLocation, FMonsterSpawnData* SelectedMonster)
{
	FActorSpawnParameters SpawnParams;
	
	ARogueAICharacter* NewMonster = GetWorld()->SpawnActor<ARogueAICharacter>(SelectedMonster->MonsterClass.Get(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	
	UE_VLOG_SPHERE(this, LogGameMode, Log, SpawnLocation, 32.0f, FColor::Blue, TEXT("MonsterType: %s\nCost:%s"), 
		*GetNameSafe(SelectedMonster->MonsterClass.Get()), *FString::SanitizeFloat(SelectedMonster->SpawnCost));
	
	// Set attributes, add buffs/debuffs, etc.
}

