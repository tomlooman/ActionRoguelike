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

	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TMap<FGameplayTag, float> ActivationCost;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Actions")
	TSoftObjectPtr<UTexture2D> Icon;

public:
	
	const FGameplayTagContainer& GetBlockedTags() const
	{
		return BlockedTags;
	}

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
	
#if WITH_EDITOR
	virtual bool ImplementsGetWorld() const override
	{
		return true;
	}
#endif
	
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
	
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	/* Play fire and forget cosmetics */
	UFUNCTION(BlueprintNativeEvent)
	void PlayBurstCosmetics();

protected:
	
	/* update client-side on logic that needs manual syncing, not used for cosmetics */
	UFUNCTION(BlueprintNativeEvent)
	void ApplyRunningState(bool bNewIsRunning);
	
	/* Activate cosmetics with a defined start/end while running */
	UFUNCTION(BlueprintNativeEvent)
	void StartLoopingCosmetics();
	
	/* Finish cosmetics with a defined start/end while running */
	UFUNCTION(BlueprintNativeEvent)
	void StopLoopingCosmetics();

	/* GameTime until the Action is available again */
	UPROPERTY(Transient)
	float CooldownUntil = 0;

	UPROPERTY(Transient, ReplicatedUsing=OnRep_IsRunning)
	bool bIsRunning = false;
	
	UFUNCTION()
	void OnRep_IsRunning();
};
