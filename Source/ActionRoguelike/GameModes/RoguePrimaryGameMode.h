// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/RogueGameMode.h"
#include "RoguePrimaryGameMode.generated.h"

struct FMonsterSpawnData;
struct FEnvQueryResult;
class UEnvQuery;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePrimaryGameMode : public ARogueGameMode
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	TObjectPtr<UEnvQuery> SpawnLocationQuery;
	
	UPROPERTY(EditDefaultsOnly, Category="Spawn System")
	TObjectPtr<UDataTable> MonsterSpawnTable;

	void SpawnQueryCompleted(TSharedPtr<FEnvQueryResult> QueryResult, FMonsterSpawnData* SelectedMonster);
	
	void OnMonsterClassLoaded(const FSoftObjectPath& LoadedObjectPath, UObject* LoadedObject, FVector SpawnLocation, FMonsterSpawnData* SelectedMonster);
	
public: 
	
	virtual void Tick(float DeltaSeconds) override;
	
	ARoguePrimaryGameMode();
};
