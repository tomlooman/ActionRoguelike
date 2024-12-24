// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RogueExplosiveBarrel.generated.h"


class URogueAttributeComponent;
class UNiagaraComponent;
class UStaticMeshComponent;
class URadialForceComponent;


UCLASS()
class ACTIONROGUELIKE_API ARogueExplosiveBarrel : public AActor
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

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<URogueAttributeComponent> AttributeComponent;

	//virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, URogueAttributeComponent* OwningComp, float NewHealth, float Delta);

	void Explode();
	
public:	
	ARogueExplosiveBarrel();
};
