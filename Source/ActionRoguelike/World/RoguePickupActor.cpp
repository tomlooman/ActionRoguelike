// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor.h"
#include "Components/SphereComponent.h"

// Note: this class has extra comments as it was part of Assignment 3 in the C++ Course


ARoguePickupActor::ARoguePickupActor()
{
	OverlapComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapComp"));
	// Always start with decent defaults, let Blueprint decide the final tweaked values
	OverlapComponent->SetSphereRadius(128.0f);
	RootComponent = OverlapComponent;
}

void ARoguePickupActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	OverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnActorOverlapped);
}

void ARoguePickupActor::OnActorOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// we do nothing here
}