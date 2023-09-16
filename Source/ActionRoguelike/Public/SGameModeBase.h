// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "Engine/DataTable.h"
#include "SGameModeBase.generated.h"

class UEnvQuery;
class UEnvQueryInstanceBlueprintWrapper;
class UCurveFloat;
class UDataTable;
class USMonsterData;


/* DataTable Row for spawning monsters in game mode  */
USTRUCT(BlueprintType)
struct FMonsterInfoRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	FMonsterInfoRow()
	{
		Weight = 1.0f;
		SpawnCost = 5.0f;
		KillReward = 20.0f;
	}

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId MonsterId;

	/* Relative chance to pick this monster */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight;

	/* Points required by gamemode to spawn this unit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SpawnCost;

	/* Amount of credits awarded to killer of this unit.  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float KillReward;
};

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

protected:

	/* All available monsters */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UDataTable> MonsterTable;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UEnvQuery> SpawnBotQuery;

	/* Curve to grant credits to spend on spawning monsters */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UCurveFloat> SpawnCreditCurve;
	
	/* Time to wait between failed attempts to spawn/buy monster to give some time to build up credits. */
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float CooldownTimeBetweenFailures;

	FTimerHandle TimerHandle_SpawnBots;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SpawnTimerInterval;

	// Read/write access as we could change this as our difficulty increases via Blueprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	int32 CreditsPerKill;

	/* Amount available to start spawning some bots immediately */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	int32 InitialSpawnCredit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI")
	bool bAutoStartBotSpawning;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ruleset")
	bool bAutoRespawnPlayer;

	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	TObjectPtr<UEnvQuery> PowerupSpawnQuery;

	/* All power-up classes used to spawn with EQS at match start */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	TArray<TSubclassOf<AActor>> PowerupClasses;

	/* Distance required between power-up spawn locations */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	float RequiredPowerupDistance;

	/* Amount of powerups to spawn during match start */
	UPROPERTY(EditDefaultsOnly, Category = "Powerups")
	int32 DesiredPowerupCount;

	UFUNCTION(BlueprintCallable)
	void StartSpawningBots();
	
	void SpawnBotTimerElapsed();

	void OnBotSpawnQueryCompleted(TSharedPtr<FEnvQueryResult> Result);

	void OnMonsterLoaded(FPrimaryAssetId LoadedId, FVector SpawnLocation);
	
	void OnPowerupSpawnQueryCompleted(TSharedPtr<FEnvQueryResult> Result);

	UFUNCTION()
	void RespawnPlayerElapsed(AController* Controller);

	// Points available to spend on spawning monsters
	float AvailableSpawnCredit;

	/* GameTime cooldown to give spawner some time to build up credits */
	float CooldownBotSpawnUntil;

	FMonsterInfoRow* SelectedMonsterRow;

	/* Allow per game mode actor pools and custom amount of primed instances */
	UPROPERTY(EditDefaultsOnly, Category= "Actor Pooling")
	TMap<TSubclassOf<AActor>, int32> ActorPoolClasses;

	void RequestPrimedActors();

public:

	virtual void OnActorKilled(AActor* VictimActor, AActor* Killer);

	ASGameModeBase();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void StartPlay() override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	UFUNCTION(Exec)
	void KillAll();
};
