// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "RogueBTDecorator_CheckHealth.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueBTDecorator_CheckHealth : public UBTDecorator
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector LowHealthKey;

	/* Threshold that is considered 'low health' (eg. 0.3 is 30% of maximum hitpoints) */
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin="0.0", ClampMax="1.0"))
	float LowHealthFraction = 0.3f;

public:

	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
