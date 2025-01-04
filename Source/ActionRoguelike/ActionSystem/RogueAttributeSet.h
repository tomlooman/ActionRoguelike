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

	FAttributeModification() {}

	FAttributeModification(FGameplayTag InAttribute,
		float InMagnitude,
		URogueActionComponent* InTargetComp,
		AActor* InInstigator,
		EAttributeModifyType InModificationType) :
			AttributeTag(InAttribute),
			TargetComp(InTargetComp),
			Instigator(InInstigator),
			Magnitude(InMagnitude),
			ModifyType(InModificationType) {}

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag AttributeTag;

	/* Attribute change Applies to this component */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<URogueActionComponent> TargetComp;

	/* The actor causing the attribute change, eg. the damage dealer */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Instigator;

	/* The "Delta" to apply to attribute */
	UPROPERTY(BlueprintReadOnly)
	float Magnitude = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	EAttributeModifyType ModifyType = EAttributeModifyType::AddDelta;
};


// Blueprint accessible delegate (this is how we "bind" indirectly in blueprint)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnAttributeChangedDynamic, float, NewValue, FAttributeModification, AppliedModification);
// The C++ delegate that is actually broadcast, and may itself call the above dynamic delegate by wrapping it in a lamdba
DECLARE_MULTICAST_DELEGATE_TwoParams(FAttributeChangedSignature, float, const FAttributeModification&);

/* Represents a single "float" Attribute which gives us greater flexibility in how its calculated due to buffs, items and permanent upgrades */
USTRUCT(BlueprintType)
struct FRogueAttribute
{
	GENERATED_BODY()

	FRogueAttribute() {}

	FRogueAttribute(float InBase) : Base(InBase) {}

	/* The base value, such as 'Strength' that was granted by the RPG class you picked, and modified permanently during gameplay (eg. +1 Str for a Level Up or a +1 permanent boost by consuming an item). This would be 'saved to disk'. */
	UPROPERTY(EditDefaultsOnly)
	float Base = 0.0f;

	/* Temporary modifier from buffs/debuffs, equipped items. This would not be 'saved to disk' as items would re-apply themselves on load */
	UPROPERTY(Transient)
	float Delta = 0.0f;

	/* No use in exposing this to blueprint directly as we have no good access to bind to it */
	FAttributeChangedSignature OnAttributeChanged;
	
	/* All game logic should get the value through here */
	float GetValue() const
	{
		// always clamp public value to zero, you could opt to make this a bool per attribute
		return FMath::Max(Base + Delta, 0.0f);
	}
};

USTRUCT(BlueprintType)
struct FRogueAttributeSet
{
	GENERATED_BODY()

	// Nothing happening here...
};


/**
 * Base set containing Health/HealthMax, useful for world gameplay actors
 */
USTRUCT(BlueprintType)
struct FRogueHealthAttributeSet : public FRogueAttributeSet
{
	GENERATED_BODY()

	FRogueHealthAttributeSet()
	{
		Health = FRogueAttribute(100);
		HealthMax = FRogueAttribute(Health.GetValue());
	}
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute Health;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute HealthMax;

};


USTRUCT(BlueprintType)
struct FRogueSurvivorAttributeSet : public FRogueHealthAttributeSet
{
	GENERATED_BODY()

	FRogueSurvivorAttributeSet()
	{
		AttackDamage = FRogueAttribute(25);
	}

	/* Base Damage value, all skills and damage use this multiplied by a damage coefficient
	 * (a percentage defaulting to 100%) to simplify balancing and scaling during play */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute AttackDamage;
};


USTRUCT(BlueprintType)
struct FRogueMonsterAttributeSet : public FRogueHealthAttributeSet
{
	GENERATED_BODY()

	FRogueMonsterAttributeSet()
	{
		AttackDamage = FRogueAttribute(10);
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute AttackDamage;
};

