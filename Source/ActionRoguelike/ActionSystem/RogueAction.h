// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
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
	FGameplayTag ActionName;

	UPROPERTY(EditDefaultsOnly, Category="Actions")
	FGameplayTagContainer GrantTags;

	UPROPERTY(EditDefaultsOnly, Category="Actions")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditDefaultsOnly, Category="Actions")
	float CooldownTime = 0.0f;

public:

	UFUNCTION(BlueprintCallable)
	URogueActionSystemComponent* GetOwningComponent() const;

	bool CanStart() const;

	bool IsRunning() const
	{
		return bIsRunning;
	}

	UFUNCTION(BlueprintNativeEvent, Category= "Actions")
	void StartAction();

	UFUNCTION(BlueprintNativeEvent, Category= "Actions")
	void StopAction();

	float GetCooldownTimeRemaining() const;

	FGameplayTag GetActionName() const
	{
		return ActionName;
	}

protected:

	/* GameTime until the Action is available again */
	UPROPERTY(Transient)
	float CooldownUntil = 0;

	UPROPERTY(Transient)
	bool bIsRunning = false;
};
