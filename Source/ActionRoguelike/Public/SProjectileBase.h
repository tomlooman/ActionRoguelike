// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SActorPoolingInterface.h"
#include "GameFramework/Actor.h"
#include "SProjectileBase.generated.h"

class USphereComponent;
class USProjectileMovementComponent;
class UNiagaraComponent;
class UAudioComponent;
class USoundCue;
class UCameraShakeBase;


/* SparseData to reduce memory footprint, see class description for URL*/
USTRUCT(BlueprintType)
struct FProjectileSparseData
{
	GENERATED_BODY()

	FProjectileSparseData()
	: ImpactShakeInnerRadius(0.f),
	ImpactShakeOuterRadius(1500.f)
	{ }
	
	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shake")
	float ImpactShakeInnerRadius;

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shake")
	float ImpactShakeOuterRadius;
};


/*
 * Example of Implementing SparseClassData, reduces memory by specifying a set of properties that won't change per-instance. More info: https://docs.unrealengine.com/en-US/sparse-class-data-in-unreal-engine/
 */
UCLASS(ABSTRACT, SparseClassDataTypes = ProjectileSparseData) // 'ABSTRACT' marks this class as incomplete, keeping this out of certain dropdowns windows like SpawnActor in Unreal Editor
class ACTIONROGUELIKE_API ASProjectileBase : public AActor, public ISActorPoolingInterface
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Effects|Shake")
	TSubclassOf<UCameraShakeBase> ImpactShake;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> ImpactVFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TObjectPtr<USoundCue> ImpactSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USProjectileMovementComponent> MoveComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> NiagaraLoopComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
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
	ASProjectileBase();

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void LifeSpanExpired() override;

	virtual void PoolBeginPlay_Implementation() override;

	virtual void PoolEndPlay_Implementation() override;
	
#if WITH_EDITORONLY_DATA
	//~ These properties are moving out to the FMySparseClassData struct:
	private:
	
	UPROPERTY()
	float ImpactShakeInnerRadius_DEPRECATED;

	UPROPERTY()
	float ImpactShakeOuterRadius_DEPRECATED;
#endif

#if WITH_EDITOR
public:
	// ~ This function transfers existing data into FMySparseClassData.
	virtual void MoveDataToSparseClassDataStruct() const override;
#endif

};
