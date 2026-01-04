// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameState.h"
#include "Net/UnrealNetwork.h"
#include "Projectiles/RogueProjectilesSubsystem.h"


void ARogueGameState::ServerCreateProjectile_Implementation(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig, AActor* InstigatorActor, uint32 NewID)
{
	URogueProjectilesSubsystem* Subsystem = GetWorld()->GetSubsystem<URogueProjectilesSubsystem>();
	Subsystem->InternalCreateProjectile(InPosition, InDirection, ProjectileConfig, InstigatorActor, NewID);
}


void ARogueGameState::BeginPlay()
{
	Super::BeginPlay();

	// Init with subsystem so we can access the world and subsystem within the struct during replication
	ProjectileData.OwningSubsystem = GetWorld()->GetSubsystem<URogueProjectilesSubsystem>();

	CoinPickupData.OwningSubsystem = GetWorld()->GetSubsystem<URoguePickupSubsystem>();
}

void ARogueGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARogueGameState, ProjectileData);
	DOREPLIFETIME(ARogueGameState, CoinPickupData);
}
