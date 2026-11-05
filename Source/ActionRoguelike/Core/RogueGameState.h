// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Pickups/RoguePickupItemReplication.h"
#include "Projectiles/RogueProjectileReplication.h"
#include "RogueGameState.generated.h"


class ARoguePlayerCharacter;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARogueGameState : public AGameStateBase
{
	GENERATED_BODY()
	
protected:
	
	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

public:

	bool HasSynchronizedServerTime() const;

	/* Observer cosmetics are best-effort. Shooter completion is delivered reliably through its action component. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastProjectileImpacts(const TArray<FProjectileUpdate>& Updates);

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
