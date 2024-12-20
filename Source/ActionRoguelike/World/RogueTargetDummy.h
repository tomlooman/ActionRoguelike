// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RogueTargetDummy.generated.h"


class URogueAttributeComponent;


UCLASS()
class ACTIONROGUELIKE_API ARogueTargetDummy : public APawn
{
	GENERATED_BODY()
	
public:	

	ARogueTargetDummy();

	virtual void PostInitializeComponents() override;

protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URogueAttributeComponent> AttributeComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> SkelMeshComp;

	UFUNCTION()
	void OnHealthChanged(AActor* InstigatorActor, URogueAttributeComponent* OwningComp, float NewHealth, float Delta);

};
