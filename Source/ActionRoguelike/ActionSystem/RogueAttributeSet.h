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
	AddModifier,

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
		float InMagnitude)
	{
		AttributeTag = InAttribute;
		Magnitude = InMagnitude;
	}

	FAttributeModification(FGameplayTag InAttribute,
		float InMagnitude,
		URogueActionComponent* InTargetComp,
		AActor* InInstigator,
		EAttributeModifyType InModificationType,
		const FGameplayTagContainer& InContextTags) :
			AttributeTag(InAttribute),
			TargetComp(InTargetComp),
			Instigator(InInstigator),
			Magnitude(InMagnitude),
			ModifyType(InModificationType),
			ContextTags(InContextTags) {}

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag AttributeTag;

	/* Attribute change Applies to this component */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<URogueActionComponent> TargetComp = nullptr;

	/* The actor causing the attribute change, eg. the damage dealer */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> Instigator = nullptr;

	/* The "Delta" to apply to attribute */
	UPROPERTY(BlueprintReadOnly)
	float Magnitude = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	EAttributeModifyType ModifyType = EAttributeModifyType::AddModifier;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer ContextTags;
};


// Blueprint accessible delegate (this is how we "bind" indirectly in blueprint)
DECLARE_DYNAMIC_DELEGATE_TwoParams(FAttributeChangedDynamicSignature, float, NewValue, FAttributeModification, AppliedModification);
// The C++ delegate that is actually broadcast, and may itself call the above dynamic delegate by wrapping it in a lamdba
DECLARE_MULTICAST_DELEGATE_TwoParams(FAttributeChangedSignature, float /*NewValue*/, const FAttributeModification&);

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
	float Modifier = 0.0f;

	/* All game logic should get the value through here */
	float GetValue() const
	{
		// always clamp public value to zero, you could opt to make this a bool per attribute
		return FMath::Max(Base + Modifier, 0.0f);
	}
};

UCLASS()
class URogueAttributeSet : public UObject
{
	GENERATED_BODY()

public:

	void InitializeAttributes(URogueActionComponent* InNewOwningComponent)
	{
		OwningComp = InNewOwningComponent;

		FillAttributeCache();
	}
	
	/* Allow additional work such as clamping Attributes based on another (eg. Health vs. HealthMax) */
	virtual void PostAttributeChanged() {};

	/* Fill local cache for quick access on each Attribute in the Set */
	void FillAttributeCache()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(ActionComponent::CacheAttributes);

		AttributeCache.Empty();

		for (TFieldIterator<FStructProperty> PropertyIt(GetClass()); PropertyIt; ++PropertyIt)
		{
			const FRogueAttribute* FoundAttribute = PropertyIt->ContainerPtrToValuePtr<FRogueAttribute>(this);

			// Build the tag "Attribute.Health" where "Health" is the variable name of the RogueAttribute we just iterated
			FString TagName = TEXT("Attribute." + PropertyIt->GetName());
			FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag(FName(TagName));

			AttributeCache.Add(AttributeTag, const_cast<FRogueAttribute*>(FoundAttribute));
		}
	}
	
	virtual bool IsSupportedForNetworking() const override
	{
		return true;
	}

	/* Fetch from properties stored inside the AttributeSet for quick access */
	TMap<FGameplayTag, FRogueAttribute*> AttributeCache;

	UPROPERTY(Transient)
	URogueActionComponent* OwningComp;
};


/**
 * Base set containing Health/HealthMax, useful for world gameplay actors
 */
UCLASS()
class URogueHealthAttributeSet : public URogueAttributeSet
{
	GENERATED_BODY()

public:
	
	URogueHealthAttributeSet()
	{
		Health = FRogueAttribute(100);
		HealthMax = FRogueAttribute(Health.GetValue());
	}

protected:
	
	UPROPERTY(ReplicatedUsing=OnRep_Health, EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute Health;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute HealthMax;

	virtual void PostAttributeChanged() override
	{
		Health.Base = FMath::Clamp(Health.Base, 0.0f, HealthMax.GetValue());
		
		// @todo: reduce Health when HealthMax is updated by triggering another attribute change
	}

	UFUNCTION()
	void OnRep_Health(FRogueAttribute OldValue);

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
};

UCLASS()
class URoguePawnAttributeSet : public URogueHealthAttributeSet
{
	GENERATED_BODY()

public:
	
	URoguePawnAttributeSet() : Super()
	{
		AttackDamage = FRogueAttribute(25);
	}

protected:
	
	/* Base Damage value, all skills and damage use this multiplied by a damage coefficient
	 * (a percentage defaulting to 100%) to simplify balancing and scaling during play */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute AttackDamage;
};

UCLASS()
class URogueSurvivorAttributeSet : public URoguePawnAttributeSet
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute Rage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute Credits;
	/*
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute Level;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	FRogueAttribute ExperiencePoints;*/
};


UCLASS()
class URogueMonsterAttributeSet : public URoguePawnAttributeSet
{
	GENERATED_BODY()

};

