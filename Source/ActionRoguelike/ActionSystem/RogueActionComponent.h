// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "InstancedStruct.h"
#include "RogueAttributeSet.h"
#include "RogueActionComponent.generated.h"

class URogueActionComponent;
class URogueAction;



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, URogueActionComponent*, OwningComp, URogueAction*, Action);


UCLASS(ClassGroup=(RogueGame), meta=(BlueprintSpawnableComponent))
class ACTIONROGUELIKE_API URogueActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UFUNCTION(BlueprintCallable)
	static URogueActionComponent* GetComponent(AActor* InActor);

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

	URogueActionComponent();

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

	UFUNCTION(Server, Reliable)
	void ServerStartAction(AActor* Instigator, FGameplayTag ActionName);

	UFUNCTION(Server, Reliable)
	void ServerStopAction(AActor* Instigator, FGameplayTag ActionName);

	/* Granted abilities at game start */
	UPROPERTY(EditAnywhere, Category = "Actions")
	TArray<TSubclassOf<URogueAction>> DefaultActions;

	UPROPERTY(Transient, BlueprintReadOnly, Replicated)
	TArray<TObjectPtr<URogueAction>> Actions;

	/* Fetch from properties stored inside the AttributeSet for quick access */
	TMap<FGameplayTag, const FRogueAttribute*> AttributeCache;

	virtual void InitializeComponent() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStopped;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
