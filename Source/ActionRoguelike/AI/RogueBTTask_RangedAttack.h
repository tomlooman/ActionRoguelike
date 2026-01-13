// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "RogueBTTask_RangedAttack.generated.h"

class ARogueProjectile;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueBTTask_RangedAttack : public UBTTaskNode
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category="AI")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	FName MuzzleSocketName;

	UPROPERTY(EditAnywhere, Category = "AI")
	TSubclassOf<ARogueProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxBulletSpread = 5.0f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	URogueBTTask_RangedAttack();
};
