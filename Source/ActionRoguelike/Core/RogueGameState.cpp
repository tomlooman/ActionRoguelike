// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameState.h"
#include "Net/UnrealNetwork.h"
#include "Projectiles/RogueProjectilesSubsystem.h"


void FProjectileConfig::PostReplicatedAdd(const FProjectileConfigArray& InArraySerializer)
{
	InArraySerializer.OwningSubsystem->InternalCreateProjectile(this->InitialPosition, InitialDirection, ConfigDataAsset, InstigatorActor, ID);
}

void FProjectileConfig::PreReplicatedRemove(const FProjectileConfigArray& InArraySerializer)
{
	// It can happen that clients already removed the projectile ID locally from its local collision checks
	InArraySerializer.OwningSubsystem->RemoveProjectileID(ID);
}

void FProjectileConfig::PostReplicatedChange(const FProjectileConfigArray& InArraySerializer)
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

// ------- END STRUCT ------ //


void ARogueGameState::BeginPlay()
{
	Super::BeginPlay();

	// Init with subsystem so we can access the world and subsystem within the struct during replication
	ProjectileData.OwningSubsystem = GetWorld()->GetSubsystem<URogueProjectilesSubsystem>();
}

void ARogueGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARogueGameState, ProjectileData);
}
