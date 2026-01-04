// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Pickups/RoguePickupItemReplication.h"
#include "Projectiles/RogueProjectileReplication.h"
#include "RogueGameState.generated.h"


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARogueGameState : public AGameStateBase
{
	GENERATED_BODY()

public:

	/* Client requests a new projectile, client will already spawn this locally and it will be kept in sync
	 * rather than waiting for the server to tell them to spawn it
	 */
	UFUNCTION(Server, reliable)
	void ServerCreateProjectile(FVector InPosition, FVector InDirection, URogueProjectileData* ProjectileConfig,
		AActor* InstigatorActor, uint32 NewID);

	virtual void BeginPlay() override;

	/*
	 * Holds all replicated "data oriented" projectiles. No Actors, just data handling all movement, cosmetics and overlaps
	 */
	UPROPERTY(Replicated)
	FProjectileArray ProjectileData;

	/* Holds the cosmetic information for clients to show pickup locations
	 * The subsystem holds the internal data to handle actual distance and pickup checks 
	 */
	UPROPERTY(Replicated)
	FPickupLocationsArray CoinPickupData;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
