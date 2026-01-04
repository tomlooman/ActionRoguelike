#pragma once

#include "RogueProjectileData.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "RogueProjectileReplication.generated.h"


class URogueProjectilesSubsystem;
struct FProjectileArray;
class UNiagaraComponent;

/*
 * Data oriented projectile data to replicate and bookkeeping for all data which we dont need every frame
 * The other array handles the per-frame data such as current position and velocity
 */
USTRUCT()
struct FProjectileItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FProjectileItem(FVector InPosition, FVector InDirection, URogueProjectileData* InDataAsset, AActor* InActor, uint32 InstanceID, float InExpirationTime):
		InitialPosition(InPosition),
		InitialDirection(InDirection),
		ConfigDataAsset(InDataAsset),
		InstigatorActor(InActor),
		ID(InstanceID),
		ExpirationGameTime(InExpirationTime)
	{
	}

	FProjectileItem() :
		InitialPosition(FVector::ZeroVector),
		InitialDirection(FVector::ForwardVector),
		ConfigDataAsset(nullptr),
		InstigatorActor(nullptr),
		ID(-1)
	{
	}

	FProjectileItem(uint32 InID) :
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
	
	void PostReplicatedAdd(const FProjectileArray& InArraySerializer);
	void PreReplicatedRemove(const FProjectileArray& InArraySerializer);
	void PostReplicatedChange(const FProjectileArray& InArraySerializer);
	
	bool operator==(const FProjectileItem& OtherConfig) const
	{
		return OtherConfig.ID == ID;
	}
};

/*
 * 
 */
USTRUCT()
struct FProjectileArray: public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(NotReplicated)
	URogueProjectilesSubsystem* OwningSubsystem = nullptr;

	UPROPERTY()
	TArray<FProjectileItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FProjectileItem, FProjectileArray>( Items, DeltaParms, *this );
	}
};

template<>
struct TStructOpsTypeTraits<FProjectileArray> : public TStructOpsTypeTraitsBase2<FProjectileArray>
{
	enum 
	{
		WithNetDeltaSerializer = true,
   };
};

