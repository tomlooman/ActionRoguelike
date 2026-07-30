// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "RogueBTTask_FindNextPatrolPoint.generated.h"

class ARoguePatrolPoint;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueBTTask_FindNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
protected:
		
	UPROPERTY(EditAnywhere, Category="Patrolling")
	FBlackboardKeySelector CurrentPatrolPointKey;
	
	/* Previous patrol point, to skip during selection of the next point */
	UPROPERTY(EditAnywhere, Category="Patrolling")
	FBlackboardKeySelector PrevPatrolPointKey;
	
	ARoguePatrolPoint* FindNearestPatrolActor(UBehaviorTreeComponent& OwnerComp);

public:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
