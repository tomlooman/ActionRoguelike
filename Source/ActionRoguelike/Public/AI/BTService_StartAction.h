// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_StartAction.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API UBTService_StartAction : public UBTService
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Action")
	FGameplayTag ActionName;
		
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

};
