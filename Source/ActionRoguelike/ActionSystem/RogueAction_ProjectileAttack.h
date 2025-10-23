// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionRoguelike.h"
#include "ActionSystem/RogueAction.h"
#include "RogueAction_ProjectileAttack.generated.h"

class UNiagaraSystem;
class UAnimMontage;
class USoundBase;
class URogueProjectileData;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueAction_ProjectileAttack : public URogueAction
{
	GENERATED_BODY()
	
public:

	virtual void StartAction_Implementation(AActor* Instigator) override;
	
protected:

	/* Not every projectile will support DoD (yet). While WIP we only support the Magic Projectile class */
	UPROPERTY(EditDefaultsOnly, Category="Performance")
	bool bSupportsDataOrientedProjectiles = false;
	
	UPROPERTY(EditDefaultsOnly)
	URogueProjectileData* ProjectileConfig;

	/* Sphere radius of the sweep to find desired target under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepRadius = 20.0f;

	/* Fallback distance when sweep finds no collision under crosshair. Adjusts final projectile direction */
	UPROPERTY(EditAnywhere, Category="Targeting")
	float SweepDistanceFallback = 5000.f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Effects")
	FName HandSocketName = MeshSockets::RightHandMuzzle;

	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float AttackAnimDelay = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	TObjectPtr<UAnimMontage> AttackAnim;

	/* Particle System played during attack animation */
	UPROPERTY(EditAnywhere, Category = "Attack")
	TObjectPtr<UNiagaraSystem> CastingEffect;

	/* Sound Effect to play (Can be Wave or Cue) */
	UPROPERTY(EditAnywhere, Category = "Attack")
	TObjectPtr<USoundBase> CastingSound;
	
	void AttackDelay_Elapsed(class ARoguePlayerCharacter* InstigatorCharacter);
};
