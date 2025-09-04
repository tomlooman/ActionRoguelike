// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueProjectile.h"
#include "RogueProjectileBlackhole.generated.h"

class URadialForceComponent;

UCLASS(Abstract)
class ACTIONROGUELIKE_API ARogueProjectileBlackhole : public ARogueProjectile
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(VisibleAnywhere, Category= "Components")
	TObjectPtr<URadialForceComponent> RadialForceComponent;

	UFUNCTION()
	void OnSphereOverlappedActor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

public:
	
	virtual void PostInitializeComponents() override;

	ARogueProjectileBlackhole();
};
