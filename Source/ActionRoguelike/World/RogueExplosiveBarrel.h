// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/RogueActionSystemInterface.h"
#include "ActionSystem/RogueAttributeSet.h"
#include "GameFramework/Actor.h"
#include "RogueExplosiveBarrel.generated.h"


class URogueActionComponent;
class UNiagaraComponent;
class UStaticMeshComponent;
class URadialForceComponent;


UCLASS()
class ACTIONROGUELIKE_API ARogueExplosiveBarrel : public AActor, public IRogueActionSystemInterface
{
	GENERATED_BODY()

protected:
	
	int32 HitCounter = 0;

	FTimerHandle DelayedExplosionHandle;

	float ExplosionDelayTime = 2.0f;

	bool bExploded = false;
	
	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<URogueActionComponent> ActionComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<URadialForceComponent> ForceComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<UNiagaraComponent> ExplosionComp;

	UPROPERTY(VisibleAnywhere, Category = Components)
	TObjectPtr<UNiagaraComponent> FlamesFXComp;

	void Explode();

	void OnHealthAttributeChanged(float NewValue, const FAttributeModification& AttributeModification);
	
public:

	virtual URogueActionComponent* GetActionComponent() const override
	{
		return ActionComp;
	}
	
	ARogueExplosiveBarrel();

	virtual void PostInitializeComponents() override;
};
