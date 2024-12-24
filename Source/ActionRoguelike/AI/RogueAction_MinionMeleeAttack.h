// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/RogueAction.h"
#include "RogueAction_MinionMeleeAttack.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class ACTIONROGUELIKE_API URogueAction_MinionMeleeAttack : public URogueAction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Animation)
	TObjectPtr<UAnimMontage> MeleeMontage;

public:

	virtual void StartAction_Implementation(AActor* Instigator) override;
	
	virtual void StopAction_Implementation(AActor* Instigator) override;

protected:

	FDelegateHandle OnOverlapHandle;

	/* Have we already applied damage once */
	bool bDamageApplied = false;

	void OnMeleeOverlaps(const TArray<FOverlapResult>& Overlaps);

public:

	URogueAction_MinionMeleeAttack();

};
