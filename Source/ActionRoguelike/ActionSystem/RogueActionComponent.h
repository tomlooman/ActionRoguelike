// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "RogueAttributeSet.h"
#include "StructUtils/InstancedStruct.h"
#include "RogueActionComponent.generated.h"

class URogueActionComponent;
class URogueAction;



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, URogueActionComponent*, OwningComp, URogueAction*, Action);


UCLASS(ClassGroup=(RogueGame), meta=(BlueprintSpawnableComponent))
class ACTIONROGUELIKE_API URogueActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	static URogueActionComponent* GetActionComponent(AActor* FromActor);

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer ActiveGameplayTags;

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void AddAction(AActor* Instigator, TSubclassOf<URogueAction> ActionClass);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	void RemoveAction(URogueAction* ActionToRemove);

	/* Returns first occurrence of action matching the class provided */
	UFUNCTION(BlueprintCallable, Category = "Actions")
	URogueAction* GetAction(TSubclassOf<URogueAction> ActionClass) const;

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StartActionByName(AActor* Instigator, FGameplayTag ActionName);

	UFUNCTION(BlueprintCallable, Category = "Actions")
	bool StopActionByName(AActor* Instigator, FGameplayTag ActionName);

	/* Stop every action, for example during death */
	void StopAllActions();

	URogueActionComponent();

	FRogueAttribute* GetAttribute(FGameplayTag InAttributeTag);

	//UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Attributes, meta = (Keywords = "Add, Set"))
	bool ApplyAttributeChange(const FAttributeModification& Modification);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = Attributes, meta = (Keywords = "Add, Set"))
	bool ApplyAttributeChange(FGameplayTag InAttributeTag, float InMagnitude, AActor* Instigator, EAttributeModifyType ModType, FGameplayTagContainer InContextTags = FGameplayTagContainer());

	/* Provide a default attribute set type for (base) classes, blueprint can set this via the details panel instead */
	void SetDefaultAttributeSet(UScriptStruct* InDefaultType);

protected:

	void FillAttributeCache();

	UFUNCTION(BlueprintCallable, Category=Attributes, DisplayName="GetAttribute")
	bool K2_GetAttribute(FGameplayTag InAttributeTag, float& CurrentValue, float& Base, float& Delta);

	/* Marked protected, C++ can use direct access to the OnAttributeChanged inside an Attribute */
	UFUNCTION(BlueprintCallable, DisplayName="AddAttributeListener", meta = (Keywords = "Bind, Delegate", AdvancedDisplay="bCallImmediately"))
	void K2_AddAttributeListener(FGameplayTag AttributeTag, FOnAttributeChangedDynamic Event, bool bCallImmediately = false);

	UFUNCTION(BlueprintCallable)
	void K2_RemoveAttributeListener(FOnAttributeChangedDynamic Event);

	/* Interchangeable set of attributes such as Health, BaseDamage, Strength, Stamina, MoveSpeed, etc. */
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintReadOnly, Category=Attributes, meta = (BaseStruct = "RogueAttributeSet", ExcludeBaseStruct))
	FInstancedStruct AttributeSet;

	/* Fetch from properties stored inside the AttributeSet for quick access */
	TMap<FGameplayTag, FRogueAttribute*> AttributeCache;

	/* List of delegates that came from Blueprint to ensure we can clean up "dead" hooks */
	TMap<FOnAttributeChangedDynamic, FDelegateHandle> DynamicDelegateHandles;

	UFUNCTION(Server, Reliable)
	void ServerStartAction(AActor* Instigator, FGameplayTag ActionName);

	UFUNCTION(Server, Reliable)
	void ServerStopAction(AActor* Instigator, FGameplayTag ActionName);

	/* Granted abilities at game start */
	UPROPERTY(EditAnywhere, Category = "Actions")
	TArray<TSubclassOf<URogueAction>> DefaultActions;

	UPROPERTY(Transient, BlueprintReadOnly, ReplicatedUsing=OnRep_Actions)
	TArray<TObjectPtr<URogueAction>> Actions;

	/* Non-replicated map for quick lookups to Actions. */
	UPROPERTY(Transient)
	TMap<FGameplayTag, TObjectPtr<URogueAction>> CachedActions;

	virtual void InitializeComponent() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnRep_Actions();

public:	

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStopped;

};
