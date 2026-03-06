// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/RogueActionSystemInterface.h"
#include "Core/RogueGameplayInterface.h"
#include "GameFramework/Actor.h"
#include "RogueTargetDummy.generated.h"

class URogueMonsterData;
struct FAttributeModification;
class URogueActionComponent;


UCLASS()
class ACTIONROGUELIKE_API ARogueTargetDummy : public APawn, public IRogueActionSystemInterface, public IRogueGameplayInterface
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<URogueActionComponent> ActionComp;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> SkelMeshComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> DefaultSceneRootComp;

	/* Index must match the CustomPrimitiveData index used in the Overlay material */
	UPROPERTY(VisibleAnywhere, Category=TargetDummy)
	int32 HitFlash_CustomPrimitiveIndex = 0;

	/* Re-use some monster data such as hit reactions */
	UPROPERTY(EditAnywhere, Category=TargetDummy)
	TObjectPtr<URogueMonsterData> ReferenceMonsterData;
	
	void OnHealthChanged(float NewValue,  const FAttributeModification& AttributeMod);
	
public:	

	ARogueTargetDummy();

	virtual void PostInitializeComponents() override;
	
	virtual bool AddImpulseAtLocationCustom(FVector Impulse, FVector Location, FName BoneName = NAME_None) override;

	virtual URogueActionComponent* GetActionComponent() const override
	{
		return ActionComp;
	}
};
