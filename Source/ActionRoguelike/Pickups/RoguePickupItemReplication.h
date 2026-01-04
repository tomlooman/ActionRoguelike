#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "RoguePickupItemReplication.generated.h"


class URoguePickupSubsystem;

USTRUCT()
struct FPickupLocationItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:

	FPickupLocationItem()
	{
		CoinLocation = FVector::ZeroVector;
		ID = FPrimitiveInstanceId();
	}

	/*FPickupLocationItem(FVector InLocation)
		: CoinLocation(InLocation), ID(INDEX_NONE)
	{
		
	}*/

	FPickupLocationItem(FVector InLocation, FPrimitiveInstanceId InID) :
		CoinLocation(InLocation),
		ID(InID) {}
	
	UPROPERTY()
	FVector_NetQuantize CoinLocation;

	/* Local ID matching the Instanced Mesh Component */
	UPROPERTY(NotReplicated)
	FPrimitiveInstanceId ID;
	
	bool operator==(const FPickupLocationItem& OtherConfig) const
	{
		return OtherConfig.ID == ID;
	}
};

USTRUCT()
struct FPickupLocationsArray: public FFastArraySerializer
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(NotReplicated)
	URoguePickupSubsystem* OwningSubsystem = nullptr;

	UPROPERTY()
	TArray<FPickupLocationItem> Items;

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);

	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FPickupLocationItem, FPickupLocationsArray>( Items, DeltaParms, *this );
	}
};

template<>
struct TStructOpsTypeTraits<FPickupLocationsArray> : public TStructOpsTypeTraitsBase2<FPickupLocationsArray>
{
	enum 
	{
		WithNetDeltaSerializer = true,
   };
};
