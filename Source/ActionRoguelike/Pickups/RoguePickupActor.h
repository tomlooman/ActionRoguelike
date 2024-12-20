// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/RogueGameplayInterface.h"
#include "RoguePickupActor.generated.h"


class USphereComponent;
class UStaticMeshComponent;


UCLASS(ABSTRACT)
class ACTIONROGUELIKE_API ARoguePickupActor : public AActor, public IRogueGameplayInterface
{
	GENERATED_BODY()

protected:

	UPROPERTY(Transient, ReplicatedUsing=OnRep_IsActive)
	bool bIsActive;

	/* Overlap sphere to automatically pickup on walkover */
	UPROPERTY(EditDefaultsOnly, Category=Powerup)
	bool bCanAutoPickup = false;

	UFUNCTION()
	void OnRep_IsActive();

	UPROPERTY(EditAnywhere, Category = Powerup)
	float RespawnTime;

	void ShowPickup();

	void HideAndCooldown();

	void SetPickupState(bool bNewIsActive);

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<UStaticMeshComponent> MeshComp;

public:

	virtual void PostInitializeComponents() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	virtual void Interact_Implementation(APawn* InstigatorPawn) override;

	virtual FText GetInteractText_Implementation(APawn* InstigatorPawn) override;

	ARoguePickupActor();

};
