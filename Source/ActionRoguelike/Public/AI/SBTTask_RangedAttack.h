// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "SBTTask_RangedAttack.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USBTTask_RangedAttack : public UBTTaskNode
{
	GENERATED_BODY()
		
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

	/* Max Random Bullet Spread (in Degrees) in positive and negative angle (shared between Yaw and Pitch) */
	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxBulletSpread;

	UPROPERTY(EditAnywhere, Category = "AI")
	TSubclassOf<AActor> ProjectileClass;

public:

	USBTTask_RangedAttack();
};
