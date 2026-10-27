// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Animation/RogueCurveAnimSubsystem.h"
#include "GameFramework/Actor.h"
#include "Core/RogueGameplayInterface.h"
#include "RogueTreasureChest.generated.h"

class UNiagaraComponent;
class UStaticMeshComponent;

UCLASS()
class ACTIONROGUELIKE_API ARogueTreasureChest : public AActor, public IRogueGameplayInterface
{
	GENERATED_BODY()

public:

	virtual void Interact_Implementation(AController* InstigatorController) override;

	virtual void OnActorLoaded_Implementation() override;

protected:

	UPROPERTY(EditDefaultsOnly, Category= "Animation")
	TObjectPtr<UCurveFloat> LidAnimCurve;

	UPROPERTY(BlueprintReadOnly, SaveGame, ReplicatedUsing=OnRep_LidOpened)
	bool bLidOpened;
	
	UFUNCTION()
	void OnRep_LidOpened();
	
	/*
	 * Store server time, for late joiners and relevancy to skip animations and cosmetic effects
	 */
	UPROPERTY(ReplicatedUsing=OnRep_InteractedTime)
	float InteractedServerTime;

	void StartOpenChestAnim();
	
	void SetChestFullyOpened();

	UFUNCTION()
	void OnRep_InteractedTime();

	UPROPERTY(VisibleAnywhere, Category=Components)
	TObjectPtr<UStaticMeshComponent> BaseMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Components)
	TObjectPtr<UStaticMeshComponent> LidMesh;

	UPROPERTY(VisibleAnywhere, Category=Components)
	TObjectPtr<UNiagaraComponent> OpenChestEffect;

	UPROPERTY(VisibleAnywhere, Category=Components)
	TObjectPtr<UAudioComponent> OpenChestSound;

public:	

	ARogueTreasureChest();
};
