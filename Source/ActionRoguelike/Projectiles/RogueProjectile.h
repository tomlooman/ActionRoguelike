// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RogueProjectile.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;
class UAudioComponent;

UCLASS(Abstract)
class ACTIONROGUELIKE_API ARogueProjectile : public AActor
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<USphereComponent> SphereComponent;
	
	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UNiagaraComponent> LoopedNiagaraComponent;

	UPROPERTY(EditDefaultsOnly, Category="Components")
	TObjectPtr<UAudioComponent> LoopedAudioComponent;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TObjectPtr<UNiagaraSystem> ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category="Sound")
	TObjectPtr<USoundBase> ExplosionSound;

	UFUNCTION()
	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void PlayExplodeEffects();
	
public:

	virtual void PostInitializeComponents() override;

	ARogueProjectile();
};
