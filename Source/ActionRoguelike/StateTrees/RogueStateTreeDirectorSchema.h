// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StateTreeComponentSchema.h"

#include "RogueStateTreeDirectorSchema.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "StateTree Director Schema"))
class ACTIONROGUELIKE_API URogueStateTreeDirectorSchema : public UStateTreeComponentSchema
{
	GENERATED_BODY()

	virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;
};
