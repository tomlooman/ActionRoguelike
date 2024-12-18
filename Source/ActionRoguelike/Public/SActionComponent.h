// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "SActionComponent.generated.h"

class USAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, USActionComponent*, OwningComp, USAction*, Action);

/* Represents a single "float" Attribute which gives us greater flexibility in how its calculated due to buffs, items and permanent upgrades */
USTRUCT(BlueprintType)
struct FRogueAttribute
{
	GENERATED_BODY()

	// Don't need to specify this tag if we just name our variables properly. eg. Attribute.Health should be named Health as a member variable in the attribute set
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Attributes")
	//FGameplayTag AttributeName;
	
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
