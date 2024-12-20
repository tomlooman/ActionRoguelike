// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "RogueAnimInstance.generated.h"

class URogueActionComponent;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:

	/* Is Pawn Stunned based on GameplayTag data */
	UPROPERTY(Transient, EditAnywhere, BlueprintReadOnly, Category = "Animation")
	bool bIsStunned;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<URogueActionComponent> ActionComp;

	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

};
