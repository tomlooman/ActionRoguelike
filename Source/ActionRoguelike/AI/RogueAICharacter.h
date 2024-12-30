// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Performance/RogueSignificanceComponent.h"
#include "GameFramework/Character.h"
#include "RogueAICharacter.generated.h"


class UNiagaraComponent;
class URogueSignificanceComponent;
class URogueAttributeComponent;
class UUserWidget;
class URogueWorldUserWidget;
class URogueActionComponent;

UCLASS()
class ACTIONROGUELIKE_API ARogueAICharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* GetTargetActor() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAttackFX();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	/* Index must match the CustomPrimitiveData index used in the Overlay material */
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	int32 HitFlash_CustomPrimitiveIndex;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	FName AttackFX_Socket;

	UPROPERTY(EditDefaultsOnly, Category="Effects")
	TObjectPtr<UAnimMontage> AttackMontage;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, URogueAttributeComponent* OwningComp, float NewHealth, float Delta);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueAttributeComponent> AttributeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueActionComponent> ActionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AttackSoundComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> AttackParticleComp;

	/* Handle fidelity for AI as they are off-screen or at far distances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueSignificanceComponent> SigManComp;

	UFUNCTION()
	void OnSignificanceChanged(ESignificanceValue Significance);

	UPROPERTY(Transient)
	TObjectPtr<URogueWorldUserWidget> ActiveHealthBar;

public:

	virtual FGenericTeamId GetGenericTeamId() const override;

	virtual void PostInitializeComponents() override;

	ARogueAICharacter();

protected:

	float CachedOverlayMaxDistance;

	FTimerHandle OverlayTimerHandle;

};
