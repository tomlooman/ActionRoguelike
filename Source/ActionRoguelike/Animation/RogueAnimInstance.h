// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RogueAnimInstance.generated.h"

class URogueActionComponent;
class USoundBase;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnAnimNofify_MeleeOverlap, const TArray<FOverlapResult>&);


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	/* Broadcast by AnimNotify_Melee anytime a melee successfully finds overlaps  */
	FOnAnimNofify_MeleeOverlap OnMeleeOverlap;

	/* Footsteps triggered via anim notifies */
	UPROPERTY(EditDefaultsOnly, Category=Audio)
	TObjectPtr<USoundBase> FootstepSound;
	
protected:

	/* Is Pawn Stunned based on GameplayTag data */
	UPROPERTY(Transient, EditAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsStunned;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<URogueActionComponent> ActionComp;

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual bool HandleNotify(const FAnimNotifyEvent& AnimNotifyEvent) override;

};
