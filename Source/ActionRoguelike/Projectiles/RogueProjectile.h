// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Performance/RogueActorPoolingInterface.h"
#include "GameFramework/Actor.h"
#include "RogueProjectile.generated.h"

class UNiagaraSystem;
class USphereComponent;
class URogueProjectileMovementComponent;
class UNiagaraComponent;
class UAudioComponent;
class USoundBase;
class UCameraShakeBase;

/*
 * 
 */
UCLASS(ABSTRACT) // 'ABSTRACT' marks this class as incomplete, keeping this out of certain dropdowns windows like SpawnActor in Unreal Editor
class ACTIONROGUELIKE_API ARogueProjectile : public AActor, public IRogueActorPoolingInterface
{
	GENERATED_BODY()

protected:
		
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shake")
	float ImpactShakeInnerRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shake")
	float ImpactShakeOuterRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shake")
	TSubclassOf<UCameraShakeBase> ImpactShake;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UNiagaraSystem> ImpactVFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<USoundBase> ImpactSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueProjectileMovementComponent> MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> NiagaraLoopComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AudioComp;

	// 'virtual' so we can override this in child-classes
	UFUNCTION()
	virtual void OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// BlueprintNativeEvent = C++ base implementation, can be expanded in Blueprints
	// BlueprintCallable to allow child classes to trigger explosions
	// Not required for assignment, useful for expanding in Blueprint later on
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void Explode();

	virtual void PostInitializeComponents() override;

public:	
	ARogueProjectile();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void LifeSpanExpired() override;

	virtual void PoolBeginPlay_Implementation() override;

	virtual void PoolEndPlay_Implementation() override;

};
