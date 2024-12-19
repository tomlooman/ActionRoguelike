// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "RogueTypes.h"
#include "SActionComponent.generated.h"

class USActionComponent;
class USAction;

/*
 * Short-lived collection of params (so we shouldn't need to keep weakobjptr's to the components)
 */
USTRUCT(BlueprintType)
struct FAttributeModification
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USActionComponent> OwningComp;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USActionComponent> InstigatorComp;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTag AttributeTag;

	UPROPERTY(BlueprintReadWrite)
	float Magnitude;

	UPROPERTY(BlueprintReadWrite)
	EAttributeModifyType ModifyType;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, USActionComponent*, OwningComp, USAction*, Action);

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


UCLASS(ClassGroup=(RogueGame), meta=(BlueprintSpawnableComponent))
class ACTIONROGUELIKE_API USActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UFUNCTION(BlueprintCallable)
	static USActionComponent* GetComponent(AActor* InActor);

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer ActiveGameplayTags;

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddAction(AActor* Instigator, TSubclassOf<USAction> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void RemoveAction(USAction* ActionToRemove);

	/* Returns first occurrence of action matching the class provided */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	USAction* GetAction(TSubclassOf<USAction> ActionClass) const;

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionByName(AActor* Instigator, FGameplayTag ActionName);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StopActionByName(AActor* Instigator, FGameplayTag ActionName);

	USActionComponent();

	bool GetAttribute(FGameplayTag InAttributeTag, FRogueAttribute& OutAttribute);

	UFUNCTION(BlueprintCallable, Category=Attributes, DisplayName="GetAttribute")
	bool K2_GetAttribute(FGameplayTag InAttributeTag, float& CurrentValue, float& Base, float& Delta);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Attributes, meta = (Keywords = "Add, Set"))
	bool ApplyAttributeChange(FGameplayTag InAttributeTag, FAttributeModification Modification);

	void BroadcastAttributeListener(FGameplayTag AttributeTag, float NewValue, const FAttributeModification& AppliedMod);

	void K2_AddAttributeListener(FGameplayTag AttributeTag, const FOnAttributeChangedDynamic& Event);

	FDelegateHandle AddAttributeListener(FGameplayTag AttributeTag, const FOnAttributeChangedNonDynamic& Func);

	void RemoveAttributeListener(FGameplayTag AttributeTag, FDelegateHandle Handle);
	
	void RemoveAttributeListener(FGameplayTag AttributeTag, FAttributeDelegateHandle Handle);

	// Keep a list of delegates per unique gameplaytag
	TMap<FGameplayTag, FOnAttributeChangedList> AttributeListeners;

	TMap<FGameplayTag, TArray<FAttributeDelegateHandle>> Listeners;

protected:
	

	/* Interchangeable set of attributes such as Health, BaseDamage, Strength, Stamina, MoveSpeed, etc. */
	UPROPERTY(EditAnywhere, Category=Attributes, meta = (BaseStruct = "RogueAttributeSet", ExcludeBaseStruct))
	FInstancedStruct AttributeSet;
	
	bool GetAttributeName(const FGameplayTag InTag, FName& OutAttributeName);

	UFUNCTION(Server, Reliable)
	void ServerStartAction(AActor* Instigator, FGameplayTag ActionName);

	UFUNCTION(Server, Reliable)
	void ServerStopAction(AActor* Instigator, FGameplayTag ActionName);

	/* Granted abilities at game start */
	UPROPERTY(EditAnywhere, Category = "Actions")
	TArray<TSubclassOf<USAction>> DefaultActions;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TArray<TObjectPtr<USAction>> Actions;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStopped;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
