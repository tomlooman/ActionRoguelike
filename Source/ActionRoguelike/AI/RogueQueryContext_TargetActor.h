// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionRoguelike.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "RogueQueryContext_TargetActor.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueQueryContext_TargetActor : public UEnvQueryContext
{
	GENERATED_BODY()
	
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

	UPROPERTY(EditDefaultsOnly, Category= "AI")
	FName TargetActorName = NAME_TargetActor;
};
