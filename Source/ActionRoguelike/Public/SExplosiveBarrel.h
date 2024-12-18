// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"


class UNiagaraComponent;
class UStaticMeshComponent;
class URadialForceComponent;


UCLASS()
class ACTIONROGUELIKE_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()


	int32 HitCounter = 0;

	FTimerHandle DelayedExplosionHandle;

	float ExplosionDelayTime;

	bool bExploded = false;

protected:

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<URadialForceComponent> ForceComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<UNiagaraComponent> ExplosionComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<UNiagaraComponent> FlamesFXComp;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void Explode();
	
public:	
	ASExplosiveBarrel();
};
