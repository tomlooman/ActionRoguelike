// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "ActionSystem/RogueActionSystemInterface.h"
#include "ActionSystem/RogueAttributeSet.h"
#include "Core/RogueGameplayInterface.h"
#include "GameFramework/Character.h"
#include "Performance/RogueSignificanceInterface.h"
#include "RogueAICharacter.generated.h"


class URogueMonsterData;
class ARogueMonsterCorpse;
class UNiagaraComponent;
class URogueWorldUserWidget;
class URogueActionComponent;

UCLASS()
class ACTIONROGUELIKE_API ARogueAICharacter : public ACharacter, public IGenericTeamAgentInterface, public IRogueSignificanceInterface, public IRogueActionSystemInterface, public IRogueGameplayInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* GetTargetActor() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackFX();

	UPROPERTY()
	TObjectPtr<ARogueMonsterCorpse> CorpseInstance;
	
	virtual bool AddImpulseAtLocationCustom(FVector Impulse, FVector Location, FName BoneName = NAME_None) override;

	virtual UDataAsset* GetActorConfigData() const override;

protected:

	UPROPERTY(EditAnywhere, Category=Config)
	TObjectPtr<URogueMonsterData> MonsterConfig;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	/* Index must match the CustomPrimitiveData index used in the Overlay material */
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	int32 HitFlash_CustomPrimitiveIndex = 0;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	FName AttackFX_Socket = FName("Muzzle_Front");

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TObjectPtr<UAnimMontage> AttackMontage;

	void OnHealthAttributeChanged(float NewValue, const FAttributeModification& AttributeModification);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueActionComponent> ActionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AttackSoundComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> AttackParticleComp;

	UPROPERTY(Transient)
	TObjectPtr<URogueWorldUserWidget> ActiveHealthBar;

	void CreateDamagePopupWidget(float DamageAmount);

public:

	virtual URogueActionComponent* GetActionComponent() const override
	{
		return ActionComp;
	}

	virtual void SignificanceLODChanged(int32 NewLOD) override;

	void OnReduceAnimationWork(class USkeletalMeshComponentBudgeted* InComponent, bool bReduce);
	
	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void PostInitializeComponents() override;

	ARogueAICharacter(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	/* Specifies a category for Significance Manager. Each unique Tag will have its own set of "Buckets" to sort and assign LODs based on distance etc. */
	UPROPERTY(EditDefaultsOnly, Category="Performance")
	FName SignificanceTag = "AICharacter";

	float CachedOverlayMaxDistance;

	FTimerHandle OverlayTimerHandle;

};