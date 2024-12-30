// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "RogueAttributeSet.generated.h"


class URogueActionComponent;

UENUM(BlueprintType)
enum class EAttributeModifyType: uint8
{
	AddDelta,

	AddBase,

	OverrideBase,

	Invalid
};


/*
 * Short-lived collection of params (so we shouldn't need to keep weakobjptr's to the components)
 */
USTRUCT(BlueprintType)
struct FAttributeModification
{
	GENERATED_BODY()

	FAttributeModification():
		Magnitude(0),
		ModifyType(EAttributeModifyType::AddDelta)
	{
	}

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<URogueActionComponent> OwningComp;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<URogueActionComponent> InstigatorComp;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag AttributeTag;

	UPROPERTY(BlueprintReadWrite)
	float Magnitude;

	UPROPERTY(BlueprintReadWrite)
	EAttributeModifyType ModifyType;
};


// Blueprint accessible delegate - Non-multicast since we create one delegate per binding
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAttributeChangedDynamic, float, NewValue, FAttributeModification, AppliedModification);

// C++ alternative for performance & easier to use in CPP
DECLARE_DELEGATE_TwoParams(FOnAttributeChangedNonDynamic, float, const FAttributeModification&);


/*
 * Wrapper similar to TimerManager to support both Dynamic (BP) and Non-dynamic ("easier"/faster in C++) Delegates 
 */
USTRUCT()
struct FAttributeDelegateHandle
{
	GENERATED_BODY()

	FAttributeDelegateHandle(const FOnAttributeChangedNonDynamic& InDelegate)
	{
		Delegate = InDelegate;
		OwnerObject = InDelegate.GetUObject();
		FuncName = InDelegate.TryGetBoundFunctionName();
	}

	FAttributeDelegateHandle(const FOnAttributeChangedDynamic& InDelegate)
	{
		DynamicDelegate = InDelegate;
		OwnerObject = InDelegate.GetUObject();
		FuncName = InDelegate.GetFunctionName();
	}

	FAttributeDelegateHandle() {}

	// "Blueprint" Delegate
	FOnAttributeChangedDynamic DynamicDelegate;

	// "C++" Delegate
	FOnAttributeChangedNonDynamic Delegate;

	void Execute(float InValue, const FAttributeModification& AppliedModification)
	{
		if (Delegate.IsBound())
		{
			Delegate.Execute(InValue, AppliedModification);
		}
		else
		{
			DynamicDelegate.Execute(InValue, AppliedModification);
		}
	}
	
	TWeakObjectPtr<const UObject> OwnerObject;

	FName FuncName;

	// Combining owner object + function name to identify the delegate
	bool operator==(const FAttributeDelegateHandle& Other) const
	{
		return (Other.OwnerObject.Get() == OwnerObject.Get() && Other.FuncName == FuncName);
	}
};


/* Represents a single "float" Attribute which gives us greater flexibility in how its calculated due to buffs, items and permanent upgrades */
USTRUCT(BlueprintType)
struct FRogueAttribute
{
	GENERATED_BODY()

	/* The base value, such as 'Strength' that was granted by the RPG class you picked, and modified permanently during gameplay (eg. +1 Str for a Level Up or a +1 permanent boost by consuming an item). This would be 'saved to disk'. */
	UPROPERTY(EditDefaultsOnly)
	float Base = 0.0f;

	/* Temporary modifier from buffs/debuffs, equipped items. This would not be 'saved to disk' as items would re-apply themselves on load */
	UPROPERTY()
	float Delta = 0.0f;
	
	/* All game logic should get the value through here */
	float GetValue() const
	{
		return Base + Delta;
	}
};

USTRUCT(BlueprintType)
struct FRogueAttributeSet
{
	GENERATED_BODY()

	// Nothing happening here...
};


USTRUCT(BlueprintType)
struct FRogueSurvivorAttributeSet : public FRogueAttributeSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute Health;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute HealthMax;

	/* Base Damage value, all skills and damage use this multiplied by a damage coefficient (a percentage defaulting to 100%) to simplify balancing and scaling during play */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute AttackDamage;
};


USTRUCT()
struct FOnAttributeChangedList
{
	GENERATED_BODY()
	
	TArray<FOnAttributeChangedNonDynamic> Delegates;
};
