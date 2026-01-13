// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "RogueEnvQueryContext_TargetActor.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueEnvQueryContext_TargetActor : public UEnvQueryContext
{
	GENERATED_BODY()

	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
