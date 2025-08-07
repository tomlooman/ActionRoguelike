// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueProjectilesSubsystem.generated.h"


class URogueProjectileData;
class UNiagaraComponent;
class UNiagaraSystem;
struct FProjectileConfig;

USTRUCT()
struct FProjectileInstance
{
	GENERATED_BODY()

	FProjectileInstance(FVector InPos, FVector InVelocity, uint32 InstanceID)
		: Position(InPos), Velocity(InVelocity), ID(InstanceID)
	{
	}

	FProjectileInstance(uint32 InID)
		: Position(FVector::ZeroVector), Velocity(FVector::ZeroVector), ID(InID)
	{
	}

	FProjectileInstance()
		: Position(FVector::ZeroVector), Velocity(FVector::ZeroVector), ID(0)
	{
	}

	UPROPERTY()
	FVector Position;

	UPROPERTY()
	FVector Velocity;

	/* ID for tracking with the Config data */
	UPROPERTY()
	uint32 ID;

	bool operator==(const FProjectileInstance& OtherInfo) const
	{
		return OtherInfo.ID == ID;
	}

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

	void InternalCreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor, uint32 NewID);

	//void RemoveProjectile(int32 Index);

	void RemoveProjectileID(uint32 IdToRemove);
	
	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

protected:

	void SpawnLoopedVFX();

	void SpawnImpactFX(const UWorld* World, const FProjectileConfig& ProjConfig, FVector ImpactPosition, FRotator ImpactRotation);
	
	TArray<FProjectileInstance> ProjectileInstances;

	/* World time of each projectile. To kill off old particles that missed all world geo */
	TArray<float> Lifetimes;

	/* Incrementing ID to track instances */
	uint32 CurrInstanceID = 0;
};
