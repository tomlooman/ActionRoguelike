// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "URogueEnvQueryContext_AlivePlayers.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API UURogueEnvQueryContext_AlivePlayers : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
