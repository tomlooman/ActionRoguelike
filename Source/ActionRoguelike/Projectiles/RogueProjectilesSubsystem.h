// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueProjectilesSubsystem.generated.h"


class URogueProjectileData;
class UNiagaraComponent;
class UNiagaraSystem;

//USTRUCT()
struct FProjectileInfo
{
	FVector Position;

	FVector Velocity;

	bool operator==(const FProjectileInfo& OtherInfo) const
	{
		return OtherInfo.Position == Position && OtherInfo.Velocity == Velocity;
	}
};

USTRUCT()
struct FProjectileFullData
{
	GENERATED_BODY()

	UPROPERTY()
	URogueProjectileData* ConfigData;

	UPROPERTY()
	AActor* InstigatorActor;

	UPROPERTY()
	UNiagaraComponent* FXComp = nullptr;

	UPROPERTY()
	FHitResult Hit;
};


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueProjectilesSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	void CreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor);

	void RemoveProjectile(int32 Index);

	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

protected:

	void SpawnImpactFX(const UWorld* World, const FProjectileFullData& BulkData, FVector ImpactPosition, FRotator ImpactRotation);

	// @todo: this will be replicated via GameState or similar location
	TArray<FProjectileInfo> ProjectileData;

	// @todo: this will be mainly locally (client / non-replicated) filled with stuff like configdata and active particle refs
	TArray<FProjectileFullData> ProjectileMetaData;

	/* World time of each projectile. To kill off old particles that missed all world geo */
	TArray<float> Lifetimes;

	// Temporary list of projectiles to be de-activated
	TArray<FProjectileFullData> RemovedProjectiles;
};
