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

	/*
	 * Spawn a new projectile immediately, on clients we send RPC to server to also spawn projectile
	 * Returns ID for client side prediction
	 */
	UFUNCTION(BlueprintCallable)
	int32 CreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor);

	void InternalCreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig,
		AActor* InstigatorActor, uint32 NewID);
	
	void RemoveProjectileID(uint32 IdToRemove);
	
	virtual void Tick(float DeltaTime) override;

	virtual TStatId GetStatId() const override;

	void SpawnImpactFX(const UWorld* World, const FProjectileConfig& ProjConfig, FVector ImpactPosition, FRotator ImpactRotation);
	
	uint32 GetUniqueProjID(FVector InPos, float InGameTime);
	
protected:

	bool HasAuthority() const;

	//UPROPERTY()
	TArray<FProjectileInstance> ProjectileInstances;
};
