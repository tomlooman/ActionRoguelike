// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RogueAttributeSet.generated.h"


USTRUCT()
struct FRogueAttribute
{
	GENERATED_BODY()

	FRogueAttribute() {}

	FRogueAttribute(float InBase)
		: Base(InBase) {}

	UPROPERTY(EditAnywhere)
	float Base = 0.0f;

	UPROPERTY(Transient)
	float Modifier = 0.0f;

	float GetValue()
	{
		return Base + Modifier;
	}
};


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueAttributeSet : public UObject
{
	GENERATED_BODY()
};


UCLASS()
class URogueHealthAttributeSet : public URogueAttributeSet
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category=Attributes)
	FRogueAttribute Health;
	
	UPROPERTY(EditAnywhere, Category=Attributes)
	FRogueAttribute HealthMax;

	URogueHealthAttributeSet();
};
