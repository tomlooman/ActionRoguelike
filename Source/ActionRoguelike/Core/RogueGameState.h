// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Projectiles/RogueProjectilesSubsystem.h"
#include "RogueGameState.generated.h"


/*
 * Data oriented projectile data to replicate and bookkeeping for all data which we dont need every frame
 * The other array handles the per-frame data such as current position and velocity
 */
USTRUCT()
struct FProjectileConfig : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FProjectileConfig(FVector InPosition, FVector InDirection, URogueProjectileData* InDataAsset, AActor* InActor, uint32 InstanceID):
		InitialPosition(InPosition),
		InitialDirection(InDirection),
		ConfigDataAsset(InDataAsset),
		InstigatorActor(InActor),
		ID(InstanceID)
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

	FProjectileConfig(uint32 InID): InitialPosition(FVector::ZeroVector), ConfigDataAsset(nullptr), InstigatorActor(nullptr),
	                                TracerEffectComp(nullptr), ID(InID)
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

	UPROPERTY(NotReplicated)
	UNiagaraComponent* TracerEffectComp = nullptr;

	UPROPERTY(NotReplicated)
	FHitResult Hit;
	
	/* ID for tracking with the instance data */
	UPROPERTY()
	uint32 ID;
	
	void PostReplicatedAdd(const struct FProjectileConfigArray& InArraySerializer);
	void PreReplicatedRemove(const struct FProjectileConfigArray& InArraySerializer);
	//void PostReplicatedChange(const struct FProjectileDataArray& InArraySerializer);
	
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
	URogueProjectilesSubsystem* OwningSubsystem;

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

	virtual void BeginPlay() override;
	
	UPROPERTY(Replicated)
	FProjectileConfigArray ProjectileData;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
