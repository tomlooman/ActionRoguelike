// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueItemChest.h"



ARogueItemChest::ARogueItemChest()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BasMeshComp"));
	BaseMeshComponent->SetCollisionProfileName("Interaction");
	RootComponent = BaseMeshComponent;

	LidMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMeshComp"));
	LidMeshComponent->SetCollisionProfileName("NoCollision");
	LidMeshComponent->SetupAttachment(BaseMeshComponent);
}

void ARogueItemChest::Interact_Implementation()
{
	// Play Animation
	SetActorTickEnabled(true);
}

void ARogueItemChest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentAnimationPitch = FMath::FInterpConstantTo(CurrentAnimationPitch, AnimationTargetPitch, DeltaTime, AnimationSpeed);
	LidMeshComponent->SetRelativeRotation(FRotator(CurrentAnimationPitch, 0.0f, 0.0f));

	if (FMath::IsNearlyEqual(CurrentAnimationPitch, AnimationTargetPitch))
	{
		// Animation Complete
		SetActorTickEnabled(false);

		ChestAnimationComplete();
	}
}

