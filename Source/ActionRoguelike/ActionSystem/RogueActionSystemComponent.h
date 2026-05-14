// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "RogueActionSystemComponent.generated.h"


struct FRogueAttribute;
class URogueAttributeSet;
class URogueAction;


UENUM(BlueprintType)
enum EAttributeModifyType
{
	Base,
	Modifier,
	OverrideBase,
	Invalid
};

// Native C++ delegate
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayTag /*AttributeTag*/, float /*NewAttributeValue*/, float /*OldAttributeValue*/);
// Blueprint delegate
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnAttributeDynamicChanged, FGameplayTag, AttributeTag, float, NewAttributeValue, float, OldAttributeValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGameplayTagCountChanged, FGameplayTag, UpdatedTag, int32, NewCount);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), HideCategories=(Navigation,Cooking,Tags))
class ACTIONROGUELIKE_API URogueActionSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	void StartAction(FGameplayTag InActionName);

	void StopAction(FGameplayTag InActionName);

	UFUNCTION(BlueprintCallable)
	void ApplyAttributeChange(FGameplayTag AttributeTag, float Delta, EAttributeModifyType ModifyType);

	FRogueAttribute* GetAttribute(FGameplayTag InAttributeTag) const;

	UFUNCTION(BlueprintCallable)
	float GetAttributeValue(FGameplayTag InAttributeTag) const;

	FOnAttributeChanged& GetAttributeListener(FGameplayTag AttributeTag);

	UFUNCTION(BlueprintCallable, DisplayName="Add Attribute Listener", meta = (Keywords="events,delegate"))
	void AddDynamicAttributeListener(FOnAttributeDynamicChanged Event, FGameplayTag AttributeTag);

	UFUNCTION(BlueprintCallable, DisplayName="Remove Attribute Listener", meta = (Keywords="events,delegate"))
	void RemoveDynamicAttributeListener(FOnAttributeDynamicChanged Event);

	virtual void InitializeComponent() override;

	void SetDefaultAttributeSet(TSubclassOf<URogueAttributeSet> AttributeSetClass);

	UFUNCTION(BlueprintCallable)
	void GrantAction(TSubclassOf<URogueAction> NewActionClass);
	
	void RemoveAction(URogueAction* ActionToRemove);
	
	void AppendActiveTags(FGameplayTagContainer NewTags);
	
	void RemoveActiveTags(FGameplayTagContainer TagsToRemove);

	UPROPERTY(BlueprintAssignable)
	FOnGameplayTagCountChanged GameplayTagUpdated;
	
	const FGameplayTagContainer& GetActiveTags() const
	{
		return ActiveGameplayTags;
	}

protected:

	FGameplayTagContainer ActiveGameplayTags;
	
	void CheckAgainstBlockedTags(const FGameplayTagContainer& NewTags);
	
	UPROPERTY(EditAnywhere, Instanced, NoClear, Category=ActionSystem)
	TObjectPtr<URogueAttributeSet> Attributes;

	TMap<FGameplayTag, FRogueAttribute*> CachedAttributes;

	TMap<FGameplayTag, FOnAttributeChanged> AttributeListeners;

	TMap<FGameplayTag, TArray<FOnAttributeDynamicChanged>> AttributeDynamicListeners;
	
	UPROPERTY()
	TArray<TObjectPtr<URogueAction>> Actions;

	UPROPERTY(EditAnywhere, Category=ActionSystem)
	TArray<TSubclassOf<URogueAction>> DefaultActions;

public:

	virtual void BeginPlay() override;

	URogueActionSystemComponent();
};