// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "RogueAIController.generated.h"

class UBehaviorTree;

UCLASS()
class ACTIONROGUELIKE_API ARogueAIController : public AAIController
{
	GENERATED_BODY()

public:

	ARogueAIController();

protected:

	UPROPERTY(EditDefaultsOnly, Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	virtual void BeginPlay() override;
};
