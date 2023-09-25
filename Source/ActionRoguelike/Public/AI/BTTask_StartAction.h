// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StartAction.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API UBTTask_StartAction : public UBTTaskNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Action")
	FGameplayTag ActionName;
			
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
