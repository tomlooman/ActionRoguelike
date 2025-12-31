// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/RogueActionSystemInterface.h"
#include "GameFramework/Actor.h"
#include "RogueTargetDummy.generated.h"

struct FAttributeModification;
class URogueActionComponent;


UCLASS()
class ACTIONROGUELIKE_API ARogueTargetDummy : public APawn, public IRogueActionSystemInterface
{
	GENERATED_BODY()
	
public:	

	ARogueTargetDummy();

	virtual void PostInitializeComponents() override;

	virtual URogueActionComponent* GetActionComponent() const override
	{
		return ActionComp;
	}

protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URogueActionComponent> ActionComp;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> SkelMeshComp;

	void OnHealthChanged(float NewValue,  const FAttributeModification& AttributeMod);

};
