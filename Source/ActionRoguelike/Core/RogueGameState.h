// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Projectiles/RogueProjectilesSubsystem.h"
#include "RogueGameState.generated.h"


struct FProjectileConfigArray;

/*
 * Data oriented projectile data to replicate and bookkeeping for all data which we dont need every frame
 * The other array handles the per-frame data such as current position and velocity
 */
USTRUCT()
struct FProjectileConfig : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FProjectileConfig(FVector InPosition, FVector InDirection, URogueProjectileData* InDataAsset, AActor* InActor, uint32 InstanceID, float InExpirationTime):
		InitialPosition(InPosition),
		InitialDirection(InDirection),
		ConfigDataAsset(InDataAsset),
		InstigatorActor(InActor),
		ID(InstanceID),
		ExpirationGameTime(InExpirationTime)
	{
	}

	FProjectileConfig() :
		InitialPosition(FVector::ZeroVector),
		InitialDirection(FVector::ForwardVector),
		ConfigDataAsset(nullptr),
		InstigatorActor(nullptr),
		ID(-1)
	{
	}

	FProjectileConfig(uint32 InID) :
		InitialPosition(FVector::ZeroVector),
		InitialDirection(FVector::ZeroVector),
		ConfigDataAsset(nullptr),
		InstigatorActor(nullptr),
		ID(InID)
	{
	}

	UPROPERTY()
	FVector InitialPosition;

	UPROPERTY()
	FVector InitialDirection;

	UPROPERTY() // @todo: can we replicate this asset pointer?
	URogueProjectileData* ConfigDataAsset;

	UPROPERTY()
	AActor* InstigatorActor;
		
	/* ID for tracking with the instance data */
	UPROPERTY()
	uint32 ID;

	/* Replicated once we have hit something @todo: does this happen in time as we also remove the item directly upon hit, maybe first we disable the projectile and delete it slightly later?? */
	UPROPERTY()
	FHitResult Hit;

	/* Check to avoid double playback of impact VFX as we can locally predict impacts and also later receive HitResult from server via replication. if we already played VFX we can skip */
	UPROPERTY(NotReplicated)
	bool bHasPlayedImpact = false;

	/* Server-side lifespan check, built from current game time + initial lifespan = expiration time */
	UPROPERTY(NotReplicated)
	float ExpirationGameTime = 0;

	UPROPERTY(NotReplicated)
	UNiagaraComponent* TracerEffectComp = nullptr;
	
	void PostReplicatedAdd(const FProjectileConfigArray& InArraySerializer);
	void PreReplicatedRemove(const FProjectileConfigArray& InArraySerializer);
	void PostReplicatedChange(const FProjectileConfigArray& InArraySerializer);
	
	bool operator==(const FProjectileConfig& OtherConfig) const
	{
		return OtherConfig.ID == ID;
	}
};

/*
 * 
 */
USTRUCT()
struct FProjectileConfigArray: public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(NotReplicated)
	URogueProjectilesSubsystem* OwningSubsystem = nullptr;

	UPROPERTY()
	TArray<FProjectileConfig> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FProjectileConfig, FProjectileConfigArray>( Items, DeltaParms, *this );
	}
};

template<>
struct TStructOpsTypeTraits<FProjectileConfigArray> : public TStructOpsTypeTraitsBase2<FProjectileConfigArray>
{
	enum 
	{
		WithNetDeltaSerializer = true,
   };
};


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARogueGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	/* Client requests a new projectile, client will already spawn this locally and it will be kept in sync
	 * rather than waiting for the server to tell them to spawn it
	 */
	UFUNCTION(Server, reliable)
	void ServerCreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig,
		AActor* InstigatorActor, uint32 NewID);

	virtual void BeginPlay() override;
	
	UPROPERTY(Replicated)
	FProjectileConfigArray ProjectileData;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
