


#include "RoguePickupItemReplication.h"
#include "RoguePickupSubsystem.h"
#include "Components/InstancedStaticMeshComponent.h"


void FPickupLocationsArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	if (AddedIndices.Num() == 0)
	{
		return;
	}
	
	TArray<FTransform> NewCoinsTMs;
	NewCoinsTMs.Reserve(AddedIndices.Num());
	
	for (int Index = 0; Index < AddedIndices.Num(); ++Index)
	{
		const FPickupLocationItem Item = Items[AddedIndices[Index]];
		NewCoinsTMs.Add(FTransform(Item.CoinLocation));
	}

	TArray<FPrimitiveInstanceId> NewIDs = OwningSubsystem->AddMeshInstances(NewCoinsTMs);

	// Map all new IDs back into the matching items, to delete them later
	for (int i = 0; i < AddedIndices.Num(); ++i)
	{
		FPickupLocationItem& Item = Items[AddedIndices[i]];
		Item.ID = NewIDs[i];
	}
}

void FPickupLocationsArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	if (RemovedIndices.Num() == 0)
	{
		return;
	}
	
	TArray<FPrimitiveInstanceId> IDsToRemove;
	IDsToRemove.Reserve(RemovedIndices.Num());
	
	for (int32& RemovedIndex : RemovedIndices)
	{
		IDsToRemove.Add(Items[RemovedIndex].ID);
	}

	check(OwningSubsystem->WorldISM);
	OwningSubsystem->WorldISM->RemoveInstancesById(IDsToRemove, false);
}