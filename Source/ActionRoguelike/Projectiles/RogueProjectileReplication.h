#pragma once

#include "RogueProjectileData.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "RogueProjectileReplication.generated.h"

class URogueProjectilesSubsystem;
struct FProjectileArray;

USTRUCT()
struct ACTIONROGUELIKE_API FProjectilePredictionKey
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 Session = 0;

	UPROPERTY()
	uint32 Sequence = 0;

	bool IsValid() const { return Session != 0 && Sequence != 0; }
	bool operator==(const FProjectilePredictionKey& Other) const { return Session == Other.Session && Sequence == Other.Sequence; }
	friend uint32 GetTypeHash(const FProjectilePredictionKey& Key) { return HashCombine(GetTypeHash(Key.Session), GetTypeHash(Key.Sequence)); }
};

/* Position and velocity describe the projectile at SimulationTime, in server world time. */
USTRUCT()
struct ACTIONROGUELIKE_API FProjectileLaunch
{
	GENERATED_BODY()

	UPROPERTY()
	int64 ID = 0;

	UPROPERTY()
	FProjectilePredictionKey PredictionKey;

	UPROPERTY()
	FVector Position = FVector::ZeroVector;

	UPROPERTY()
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY()
	float SimulationTime = 0;

	UPROPERTY()
	float ExpirationTime = 0;

	UPROPERTY()
	TObjectPtr<URogueProjectileData> ConfigDataAsset = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> InstigatorActor = nullptr;
};

UENUM()
enum class EProjectileOutcome : uint8
{
	Flying,
	Impacted,
	Expired,
	Rejected,
	Cancelled
};

/* Also used for owner-only responses. Impact presentation does not require a live hit actor. */
USTRUCT()
struct ACTIONROGUELIKE_API FProjectileUpdate
{
	GENERATED_BODY()

	UPROPERTY()
	FProjectileLaunch Projectile;

	UPROPERTY()
	EProjectileOutcome Outcome = EProjectileOutcome::Flying;

	UPROPERTY()
	FVector ImpactPosition = FVector::ZeroVector;

	UPROPERTY()
	FVector ImpactNormal = FVector::UpVector;

	UPROPERTY()
	bool bSpawnDecal = false;
};

USTRUCT()
struct ACTIONROGUELIKE_API FProjectileItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FProjectileLaunch Projectile;

	void PostReplicatedAdd(const FProjectileArray& InArraySerializer);
	void PreReplicatedRemove(const FProjectileArray& InArraySerializer);
	void PostReplicatedChange(const FProjectileArray& InArraySerializer);
};

USTRUCT()
struct ACTIONROGUELIKE_API FProjectileArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(NotReplicated)
	TObjectPtr<URogueProjectilesSubsystem> OwningSubsystem = nullptr;

	UPROPERTY()
	TArray<FProjectileItem> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);
	void PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters);
};

template<>
struct TStructOpsTypeTraits<FProjectileArray> : public TStructOpsTypeTraitsBase2<FProjectileArray>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};
