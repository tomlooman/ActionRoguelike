// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Performance/RogueSignificanceComponent.h"
#include "GameFramework/Character.h"
#include "RogueAICharacter.generated.h"


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
	void OnHealthChanged(AActor* InstigatorActor, URogueAttributeComponent* OwningComp, float NewHealth, float Delta);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueAttributeComponent> AttributeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueActionComponent> ActionComp;

	/* Handle fidelity for AI as they are off-screen or at far distances */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueSignificanceComponent> SigManComp;

	UFUNCTION()
	void OnSignificanceChanged(ESignificanceValue Significance);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPawnSeen();
	
	UPROPERTY(Transient)
	TObjectPtr<URogueWorldUserWidget> ActiveHealthBar;

public:

	virtual FGenericTeamId GetGenericTeamId() const;

	virtual void PostInitializeComponents() override;

	ARogueAICharacter();

};
