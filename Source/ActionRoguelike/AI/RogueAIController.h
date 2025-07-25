// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/Blackboard/BlackboardKey.h"
#include "RogueAIController.generated.h"


class UBehaviorTree;
class UAIPerceptionComponent;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API ARogueAIController : public AAIController
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

	EBlackboardNotificationResult OnTargetActorChanged(const UBlackboardComponent& Comp, FBlackboard::FKey KeyID);

	virtual void BeginPlay() override;

	virtual void PreRegisterAllComponents() override;

public:

	ARogueAIController();
};
