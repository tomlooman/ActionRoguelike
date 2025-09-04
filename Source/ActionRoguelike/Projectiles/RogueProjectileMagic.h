// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueProjectile.h"
#include "RogueProjectileMagic.generated.h"

UCLASS(Abstract)
class ACTIONROGUELIKE_API ARogueProjectileMagic : public ARogueProjectile
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category="Damage")
	TSubclassOf<UDamageType> DmgTypeClass;

	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

public:

	ARogueProjectileMagic();
};
