// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RogueTargetDummy.generated.h"

class URogueActionComponent;


UCLASS()
class ACTIONROGUELIKE_API ARogueTargetDummy : public APawn
{
	GENERATED_BODY()
	
public:	

	ARogueTargetDummy();

	virtual void PostInitializeComponents() override;

protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URogueActionComponent> ActionComp;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> SkelMeshComp;

	void OnHealthChanged(float NewValue,  const FAttributeModification& AttributeMod);

};
