// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueProjectile.h"
#include "RogueProjectile_Blackhole.generated.h"

class URadialForceComponent;

UCLASS(Abstract)
class ACTIONROGUELIKE_API ARogueProjectile_Blackhole : public ARogueProjectile
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleDefaultsOnly, Category=Components)
	TObjectPtr<URadialForceComponent> RadialForceComp;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	UCurveFloat* AnimRadiusCurve;

	UPROPERTY(EditDefaultsOnly, Category=Animation)
	float MaxRadius;

	UFUNCTION()
	void OnOverlappedPhysicsActor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;

	ARogueProjectile_Blackhole();
};
