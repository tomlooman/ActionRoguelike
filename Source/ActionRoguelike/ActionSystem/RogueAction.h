// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "RogueAction.generated.h"

class UWorld;
class URogueActionComponent;


USTRUCT()
struct FActionRepData
{
	GENERATED_BODY()

public:

	UPROPERTY()
	bool bIsRunning;

	UPROPERTY()
	TObjectPtr<AActor> Instigator;

	FActionRepData()
	{
		bIsRunning = false;
	}
};


/**
 * 
 */
UCLASS(Blueprintable)
class ACTIONROGUELIKE_API URogueAction : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "UI")
	TSoftObjectPtr<UTexture2D> GetIcon() const;

	UFUNCTION(BlueprintCallable, Category = "Action")
	URogueActionComponent* GetOwningComponent() const;

protected:
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(Transient, Replicated)
	TObjectPtr<URogueActionComponent> ActionComp;

	/* Tags added to owning actor when activated, removed when action stops */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer GrantsTags;

	/* Action can only start if OwningActor has none of these Tags applied */
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(Transient, ReplicatedUsing="OnRep_RepData")
	FActionRepData RepData;

	UPROPERTY(Transient, Replicated)
	float TimeStarted;

	UFUNCTION()
	void OnRep_RepData();

	/* Start immediately when added to an action component */
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	bool bAutoStart;

	/* Action nickname to start/stop without a reference to the object */
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	FGameplayTag ActivationTag;
	
public:

	UFUNCTION(BlueprintPure)
	FGameplayTag GetActivationTag() const
	{
		return ActivationTag;
	}

	UFUNCTION(BlueprintPure)
	bool IsAutoStart() const
	{
		return bAutoStart;
	}

	void Initialize(URogueActionComponent* NewActionComp);

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsRunning() const;

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Action")
	void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Action")
	void StopAction(AActor* Instigator);

	virtual UWorld* GetWorld() const override;

	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}
};
