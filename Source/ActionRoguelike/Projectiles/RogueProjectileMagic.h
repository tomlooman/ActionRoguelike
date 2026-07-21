// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueProjectile.h"
#include "RogueProjectileMagic.generated.h"

class URogueActionEffect;

UCLASS(Abstract)
class ACTIONROGUELIKE_API ARogueProjectileMagic : public ARogueProjectile
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float AttackDamage = 51.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	float ImpulseIntensity = 200000.f;
	
	/* Debuff to apply to the HitActor */
	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<URogueActionEffect> EffectOnHit;

	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

public:

	ARogueProjectileMagic();
};
