// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueProjectileReplication.h"
#include "Subsystems/WorldSubsystem.h"
#include "RogueProjectilesSubsystem.generated.h"

class UNiagaraComponent;
class URogueActionComponent;

/* Per-machine simulation and presentation. This data never belongs in the replicated array. */
USTRUCT()
struct FProjectileInstance
{
	GENERATED_BODY()

	UPROPERTY()
	FProjectileLaunch Launch;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> TracerEffectComp = nullptr;

	UPROPERTY()
	TWeakObjectPtr<URogueActionComponent> RequestingComponent;

	float SimulationTime = 0;
	float TentativeHitTime = 0;
	float NextStatusRequestTime = 0;
	bool bTentativeHit = false;
	bool bIgnorePredictedCollision = false;
};

USTRUCT()
struct FProjectileOwnerUpdate
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<URogueActionComponent> Owner;

	UPROPERTY()
	FProjectileUpdate Update;
};

UCLASS()
class ACTIONROGUELIKE_API URogueProjectilesSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:

	bool HasSynchronizedTime() const;
	uint32 AllocatePredictionSession();

	/* Native/server and Blueprint spawners use this; player prediction requests go through the action component. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int64 CreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor);

	FProjectileLaunch LaunchProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig,
		AActor* InstigatorActor, FProjectilePredictionKey Key, URogueActionComponent* RequestingComponent);
	void PredictProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig,
		AActor* InstigatorActor, FProjectilePredictionKey Key, URogueActionComponent* RequestingComponent);
	void ReceiveLaunch(const FProjectileLaunch& Launch, URogueActionComponent* RequestingComponent = nullptr);
	void ReceiveUpdate(const FProjectileUpdate& Update, URogueActionComponent* RequestingComponent = nullptr);
	void ReceiveRemoval(const FProjectileLaunch& Launch);
	void CancelPredictions(AActor* InstigatorActor);
	void QueryProjectileStatus(int64 ID, URogueActionComponent* RequestingComponent);
	void QueueOwnerUpdate(URogueActionComponent* Owner, const FProjectileUpdate& Update);

	const FProjectileInstance* FindProjectile(int64 ID) const;
	const FProjectileInstance* FindPrediction(FProjectilePredictionKey Key) const;
	int32 GetActiveProjectileCount() const { return ProjectileInstances.Num(); }
	int32 GetCompletedProjectileCount() const { return CompletedProjectiles.Num(); }
	int32 GetPendingLaunchCount() const { return PendingLaunches.Num(); }
	int32 GetImpactPlaybackCount() const { return ImpactPlaybackCount; }

	virtual void Tick(float DeltaTime) override;
	virtual void Deinitialize() override;
	virtual TStatId GetStatId() const override;

protected:

	bool HasAuthority() const;
	float GetServerTime() const;
	void StartTracer(FProjectileInstance& Instance);
	void StopTracer(FProjectileInstance& Instance);
	void RemoveLocalProjectile(const FProjectileLaunch& Launch);
	void FinishProjectile(int64 ID, const FHitResult* Hit);
	void SpawnImpactFX(const FProjectileUpdate& Update);
	void FlushOwnerUpdates();

	UPROPERTY(Transient)
	TArray<FProjectileInstance> ProjectileInstances;

	UPROPERTY(Transient)
	TMap<int64, FProjectileLaunch> PendingLaunches;

	UPROPERTY(Transient)
	TMap<int64, FProjectileUpdate> PendingImpacts;

	UPROPERTY(Transient)
	TMap<int64, FProjectileUpdate> CompletedProjectiles;

	UPROPERTY(Transient)
	TArray<FProjectileOwnerUpdate> PendingOwnerUpdates;

	UPROPERTY(Transient)
	TArray<FProjectileUpdate> PendingObserverUpdates;

	TSet<int64> PlayedImpacts;
	TMap<FProjectilePredictionKey, float> RejectedPredictions;
	int64 NextProjectileID = 1;
	uint32 NextPredictionSession = 1;
	int32 ImpactPlaybackCount = 0;
};
