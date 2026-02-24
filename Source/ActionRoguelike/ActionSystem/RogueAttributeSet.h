// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RogueAttributeSet.generated.h"


class URogueActionSystemComponent;

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
UCLASS(EditInlineNew)
class ACTIONROGUELIKE_API URogueAttributeSet : public UObject
{
	GENERATED_BODY()

public:
		
	URogueActionSystemComponent* GetOwningComponent() const;

	virtual void InitializeAttributes() {};

	virtual void PostAttributeChanged() {};
};


UCLASS()
class URogueHealthAttributeSet : public URogueAttributeSet
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category=Health)
	FRogueAttribute Health;
	
	UPROPERTY(EditAnywhere, Category=Health)
	FRogueAttribute HealthMax;

	virtual void PostAttributeChanged() override;

	URogueHealthAttributeSet();
};

UCLASS()
class URoguePawnAttributeSet : public URogueHealthAttributeSet
{
	GENERATED_BODY()

public:

	virtual void PostAttributeChanged() override;

	virtual void InitializeAttributes() override;

	void ApplyMoveSpeed();

	/*
	* Walking speed directly linked with Character Movement Component
	*/
	UPROPERTY(EditAnywhere, Category=MoveSpeed)
	FRogueAttribute MoveSpeed;

	UPROPERTY(EditAnywhere, Category=MoveSpeed)
	FRogueAttribute MoveSpeedMultiplier;
	
	URoguePawnAttributeSet();
};

UCLASS()
class URoguePlayerAttributeSet : public URoguePawnAttributeSet
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category=Resources)
	FRogueAttribute Rage;
};

UCLASS()
class URogueMonsterAttributeSet : public URoguePawnAttributeSet
{
	GENERATED_BODY()

public:

	URogueMonsterAttributeSet();
};