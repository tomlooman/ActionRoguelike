// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueProjectile.h"
#include "GameplayTagContainer.h"
#include "RogueProjectile_Magic.generated.h"

class URogueActionEffect;


UCLASS(Abstract)
class ACTIONROGUELIKE_API ARogueProjectile_Magic : public ARogueProjectile
{
	GENERATED_BODY()

public:

	ARogueProjectile_Magic();
	
	UPROPERTY(EditDefaultsOnly, Category = "Damage", meta=(Units="Percent"))
	float DamageCoefficient;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGameplayTag ParryTag;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<URogueActionEffect> BurningActionClass;
	
protected:

	UFUNCTION()
	void OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void PostInitializeComponents() override;

};
