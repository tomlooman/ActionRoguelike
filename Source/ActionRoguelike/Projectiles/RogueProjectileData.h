// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RogueProjectileData.generated.h"

class UNiagaraSystem;

/**
 * Projectile Configuration for Data-oriented implementation of Projectiles
 * (Actorless) purely handled through simple data arrays in RogueProjectileSubsystem
 */
UCLASS()
class ACTIONROGUELIKE_API URogueProjectileData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditDefaultsOnly, Category= "Projectile")
	UNiagaraSystem* ProjectileEffect;

	/* Note: Usually this would have multiple options based on hit PhysicsMaterial such as blood impact for hitting enemies */
	UPROPERTY(EditDefaultsOnly, Category= "Projectile")
	UNiagaraSystem* ImpactEffect;

	UPROPERTY(EditDefaultsOnly, Category= "Projectile")
	float InitialSpeed = 2000.0f;

	UPROPERTY(EditDefaultsOnly, Category= "Projectile")
	float GravityScale = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category= "Projectile")
	float Lifespan = 20.0f;

	/* Use Instigator's AttackDamage attribute multiplied by this coefficient as a percentage */
	UPROPERTY(EditDefaultsOnly, Category= "Projectile", meta=(Units="Percent"))
	float DamageCoefficient = 100;
};
