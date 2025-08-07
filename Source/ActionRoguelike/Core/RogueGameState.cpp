// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameState.h"
#include "Net/UnrealNetwork.h"
#include "Projectiles/RogueProjectilesSubsystem.h"


void FProjectileConfig::PostReplicatedAdd(const struct FProjectileConfigArray& InArraySerializer)
{
	InArraySerializer.OwningSubsystem->InternalCreateProjectile(this->InitialPosition, InitialDirection, ConfigDataAsset, InstigatorActor, ID);
}

void FProjectileConfig::PreReplicatedRemove(const struct FProjectileConfigArray& InArraySerializer)
{
	InArraySerializer.OwningSubsystem->RemoveProjectileID(ID);
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
