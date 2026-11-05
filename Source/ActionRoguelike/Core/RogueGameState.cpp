// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameState.h"
#include "Net/UnrealNetwork.h"
#include "Pickups/RoguePickupSubsystem.h"
#include "Projectiles/RogueProjectilesSubsystem.h"


bool ARogueGameState::HasSynchronizedServerTime() const
{
	return HasAuthority() || NumServerWorldTimeSecondsDeltas > 0;
}


void ARogueGameState::MulticastProjectileImpacts_Implementation(const TArray<FProjectileUpdate>& Updates)
{
	URogueProjectilesSubsystem* Subsystem = GetWorld()->GetSubsystem<URogueProjectilesSubsystem>();
	for (const FProjectileUpdate& Update : Updates)
	{
		Subsystem->ReceiveUpdate(Update);
	}
}


void ARogueGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Initialize before initial replication callbacks, rather than waiting for BeginPlay.
	ProjectileData.OwningSubsystem = GetWorld()->GetSubsystem<URogueProjectilesSubsystem>();
}


void ARogueGameState::BeginPlay()
{
	Super::BeginPlay();
	CoinPickupData.OwningSubsystem = GetWorld()->GetSubsystem<URoguePickupSubsystem>();
	if (!HasAuthority())
	{
		for (const FProjectileItem& Item : ProjectileData.Items)
		{
			ProjectileData.OwningSubsystem->ReceiveLaunch(Item.Projectile);
		}
	}
}

void ARogueGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARogueGameState, ProjectileData);
	DOREPLIFETIME(ARogueGameState, CoinPickupData);
}
