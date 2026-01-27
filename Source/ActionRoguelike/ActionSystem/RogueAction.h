// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RogueAction.generated.h"

class URogueActionSystemComponent;

/**
 * 
 */
UCLASS(Blueprintable, Abstract)
class ACTIONROGUELIKE_API URogueAction : public UObject
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	FName ActionName = FName("PrimaryAttack");

public:
	
	URogueActionSystemComponent* GetOwningComponent() const;
	
	virtual void StartAction();
	
	FName GetActionName() const
	{
		return ActionName;
	}
};
