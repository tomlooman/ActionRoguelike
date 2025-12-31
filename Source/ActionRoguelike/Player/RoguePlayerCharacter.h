// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "GenericTeamAgentInterface.h"
// Included for struct FInputActionInstance (Enhanced Input)
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "ActionSystem/RogueActionSystemInterface.h"
#include "RoguePlayerCharacter.generated.h"

struct FAttributeModification;
class URoguePlayerData;
class ARogueAIController;
class UAIPerceptionStimuliSourceComponent;
class UInputMappingContext;
class UCameraComponent;
class USpringArmComponent;
class UAnimMontage;
class URogueActionComponent;

UCLASS(Abstract)
class ACTIONROGUELIKE_API ARoguePlayerCharacter : public ACharacter, public IGenericTeamAgentInterface, public IRogueActionSystemInterface
{
	GENERATED_BODY()

public:

	void PlayAttackSound(USoundBase* InSound);

protected:

	/* Contains most config data for the player including abilities and inputs */
	UPROPERTY(EditDefaultsOnly, Category="Config")
	TObjectPtr<URoguePlayerData> PlayerConfig;

	/* Index must match the CustomPrimitiveData index used in the Overlay material */
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	int32 HitFlash_CustomPrimitiveIndex = 0;

	UPROPERTY(EditDefaultsOnly, Category= Effects)
	TObjectPtr<USoundBase> TakeDamageVOSound;
	
	UPROPERTY(EditDefaultsOnly, Category= Effects)
	TObjectPtr<USoundBase> DeathVOSound;

	UPROPERTY(EditDefaultsOnly, Category= Effects)
	TObjectPtr<USoundBase> DeathUISound;

	UPROPERTY(EditDefaultsOnly, Category= Effects)
	TObjectPtr<USoundBase> FootPlantSound;

	/* Plays on DefaultSlot, make sure the "auto blendout" is disabled on the AnimSequence to keep the final pose  */
	UPROPERTY(EditDefaultsOnly, Category= Effects)
	TObjectPtr<UAnimMontage> DeathMontage;

	/* Widget to display when bot first sees a player. */
	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UUserWidget> SpottedWidgetClass;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<URogueActionComponent> ActionComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAIPerceptionStimuliSourceComponent> PerceptionStimuliComp;

	/* Re-usable audio component for all (attached) attack sounds such as casting sound from the magic projectile attack */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UAudioComponent> AttackSoundsComp;

	// Enhanced Input
	// Three parameter options available (FInputActionInstance, FInputActionValue, or none)
	void Move(const FInputActionInstance& Instance);

	void LookMouse(const FInputActionValue& InputValue);
	
	void LookStick(const FInputActionValue& InputValue);

	void StartActionByTag(const FInputActionValue& Instance, const FGameplayTag InActionTag);
	
	void StopActionByTag(const FInputActionValue& Instance, const FGameplayTag InActionTag);

	void OnHealthAttributeChanged(float NewValue, const FAttributeModification& AttributeModification);

	virtual void PostInitializeComponents() override;

	void FindCrosshairTarget();

	void CrosshairTraceComplete(const FTraceHandle& InTraceHandle, FTraceDatum& InTraceDatum);

public:	

	ARoguePlayerCharacter();

	virtual FVector GetPawnViewLocation() const override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual FGenericTeamId GetGenericTeamId() const override;

	UFUNCTION(Client, Unreliable)
	void ClientOnSeenBy(ARogueAICharacter* SeenByPawn);

	virtual URogueActionComponent* GetActionComponent() const override
	{
		return ActionComp;
	}

protected:

	float CachedOverlayMaxDistance;

	FTimerHandle OverlayTimerHandle;

	FTraceHandle TraceHandle;
	
	bool bHasPawnTarget;
};
