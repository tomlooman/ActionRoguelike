// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/RogueInteractionInterface.h"
#include "GameFramework/Actor.h"
#include "RogueItemChest.generated.h"

class UStaticMeshComponent;

UCLASS()
class ACTIONROGUELIKE_API ARogueItemChest : public AActor, public IRogueInteractionInterface
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> BaseMeshComponent;
	
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> LidMeshComponent;

	UPROPERTY(EditAnywhere, Category="Animation")
	float AnimationSpeed = 50.0f;

	UPROPERTY(EditAnywhere, Category="Animation")
	float AnimationTargetPitch = 120.f;

	float CurrentAnimationPitch = 0.0f;

	UFUNCTION(BlueprintImplementableEvent)
	void ChestAnimationComplete();

public:

	virtual void Interact_Implementation() override;

	virtual void Tick(float DeltaTime) override;
                                           
    ARogueItemChest();
};
