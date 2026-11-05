#include "RogueProjectileReplication.h"

#include "RogueProjectilesSubsystem.h"


void FProjectileItem::PostReplicatedAdd(const FProjectileArray& InArraySerializer)
{
	if (InArraySerializer.OwningSubsystem)
	{
		InArraySerializer.OwningSubsystem->ReceiveLaunch(Projectile);
	}
}


void FProjectileItem::PreReplicatedRemove(const FProjectileArray& InArraySerializer)
{
	if (InArraySerializer.OwningSubsystem)
	{
		InArraySerializer.OwningSubsystem->ReceiveRemoval(Projectile);
	}
}


void FProjectileItem::PostReplicatedChange(const FProjectileArray& InArraySerializer)
{
	PostReplicatedAdd(InArraySerializer);
}


bool FProjectileArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FastArrayDeltaSerialize<FProjectileItem, FProjectileArray>(Items, DeltaParms, *this);
}


void FProjectileArray::PostReplicatedReceive(const FFastArraySerializer::FPostReplicatedReceiveParameters& Parameters)
{
	// Retry deferred launches when Iris/legacy replication resolves an asset or actor reference.
	if (OwningSubsystem)
	{
		for (const FProjectileItem& Item : Items)
		{
			OwningSubsystem->ReceiveLaunch(Item.Projectile);
		}
	}
}
