// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Components/SSignificanceComponent.h"
#include "GameFramework/Character.h"
#include "SAICharacter.generated.h"


class USSignificanceComponent;
class USAttributeComponent;
class UUserWidget;
class USWorldUserWidget;
class USActionComponent;

UCLASS()
class ACTIONROGUELIKE_API ASAICharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category = "AI")
	AActor* GetTargetActor() const;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> HealthBarWidgetClass;

	/* Widget to display when bot first sees a player. */
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> SpottedWidgetClass;

	/* Index must match the CustomPrimitiveData index used in the Overlay material */
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	int32 HitFlash_CustomPrimitiveIndex;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USAttributeComponent> AttributeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USActionComponent> ActionComp;

	/* Handle fidelity for AI as they are off-screen or at far distances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USSignificanceComponent> SigManComp;

	UFUNCTION()
	void OnSignificanceChanged(ESignificanceValue Significance);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPawnSeen();
	
	UPROPERTY(Transient)
	TObjectPtr<USWorldUserWidget> ActiveHealthBar;

public:

	virtual FGenericTeamId GetGenericTeamId() const;

	virtual void PostInitializeComponents() override;

	ASAICharacter();

};
