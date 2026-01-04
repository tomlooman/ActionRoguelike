#include "RogueProjectileReplication.h"

#include "RogueProjectilesSubsystem.h"


void FProjectileItem::PostReplicatedAdd(const FProjectileArray& InArraySerializer)
{
	InArraySerializer.OwningSubsystem->InternalCreateProjectile(this->InitialPosition, InitialDirection, ConfigDataAsset, InstigatorActor, ID);
}


void FProjectileItem::PreReplicatedRemove(const FProjectileArray& InArraySerializer)
{
	// It can happen that clients already removed the projectile ID locally from its local collision checks
	InArraySerializer.OwningSubsystem->RemoveProjectileID(ID);
}


void FProjectileItem::PostReplicatedChange(const FProjectileArray& InArraySerializer)
{
	// Check if hit was set
	if (Hit.GetActor() && !bHasPlayedImpact)
	{
		bHasPlayedImpact = true;
		
		FRotator ImpactRotation = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal().Rotation();
		UWorld* World = InArraySerializer.OwningSubsystem->GetWorld();

		InArraySerializer.OwningSubsystem->SpawnImpactFX(World, *this, Hit.Location, ImpactRotation);
	}
		
	//DrawDebugDirectionalArrow(World, Hit.Location, (Hit.Location + (ImpactRotation.Vector() * 50.0f)), 20.0f, FColor::Green, false, 5.0f);
	//DrawDebugSphere(World, Hit.Location, 32.0f, 32, FColor::Orange, false, 5.f);
}