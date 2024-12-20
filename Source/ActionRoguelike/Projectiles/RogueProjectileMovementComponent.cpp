// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/RogueProjectileMovementComponent.h"
#include "Performance/RogueTickablesSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueProjectileMovementComponent)


void URogueProjectileMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// BeginPlay on the component will be called after the tick has been registered via the owning Actor
	URogueTickablesSubsystem* Tickables = GetWorld()->GetSubsystem<URogueTickablesSubsystem>();
	Tickables->RegisterComponent(&PrimaryComponentTick);
}


void URogueProjectileMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	URogueTickablesSubsystem* Tickables = GetWorld()->GetSubsystem<URogueTickablesSubsystem>();
	Tickables->DeRegisterComponent(&PrimaryComponentTick);
}


void URogueProjectileMovementComponent::Reset()
{
	// Reset to default for object pooling support
	Velocity = FVector(1,0,0);

	// Properly resets velocities and rotations
	InitializeComponent();
}