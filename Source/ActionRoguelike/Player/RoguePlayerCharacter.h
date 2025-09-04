// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RoguePlayerCharacter.generated.h"

class ARogueProjectile;
class UNiagaraSystem;
class ARogueProjectileMagic;
struct FInputActionInstance;
struct FInputActionValue;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;

UCLASS()
class ACTIONROGUELIKE_API ARoguePlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARoguePlayerCharacter();

protected:

	UPROPERTY(EditDefaultsOnly, Category="PrimaryAttack")
	TSubclassOf<ARogueProjectile> PrimaryAttackProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="PrimaryAttack")
	TSubclassOf<ARogueProjectile> SecondaryAttackProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="PrimaryAttack")
	TSubclassOf<ARogueProjectile> SpecialAttackProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="PrimaryAttack")
	TObjectPtr<UNiagaraSystem> CastingEffect;

	UPROPERTY(EditDefaultsOnly, Category="PrimaryAttack")
	TObjectPtr<USoundBase> CastingSound;

	UPROPERTY(VisibleAnywhere, Category="PrimaryAttack")
	FName MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, Category="PrimaryAttack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_Move;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_Look;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_Jump;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_PrimaryAttack;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_SecondaryAttack;
	
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UInputAction> Input_SpecialAttack;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& InValue);
	
	void Look(const FInputActionInstance& InValue);

	void StartProjectileAttack(TSubclassOf<ARogueProjectile> ProjectileClass);

	void AttackTimerElapsed(TSubclassOf<ARogueProjectile> ProjectileClass);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
