// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/Character.h"
#include "RogueAICharacter.generated.h"

struct FGameplayTag;
class URogueMonsterData;
class URogueActionSystemComponent;

UCLASS(Abstract)
class ACTIONROGUELIKE_API ARogueAICharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<URogueActionSystemComponent> ActionSystemComponent;
	
	UFUNCTION()
	void OnGameplayTagUpdated(FGameplayTag UpdatedTag, int32 NewCount);
	
	UPROPERTY(EditDefaultsOnly, Category=Animation)
	TObjectPtr<UAnimMontage> StunnedAnimation;
	
	UPROPERTY(Transient)
	TObjectPtr<URogueMonsterData> MonsterData;

public:
	
	virtual FGenericTeamId GetGenericTeamId() const;
	
	URogueActionSystemComponent* GetActionSystemComponent() const
	{
		return ActionSystemComponent;
	}
	
	URogueMonsterData* GetMonsterData() const
	{
		return MonsterData;
	}
	
	void SetMonsterData(URogueMonsterData* NewMonsterData)
	{
		check(MonsterData == nullptr);
		MonsterData = NewMonsterData;
	}

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	ARogueAICharacter();

	virtual void PostInitializeComponents() override;
	
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

	FTimerHandle OverlayTimerHandle;
};
