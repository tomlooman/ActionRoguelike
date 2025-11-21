// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueHealthPickup.h"
#include "ActionSystem/RogueActionSystemComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

// Note: this class has extra comments as it was part of Assignment 3 in the C++ Course


ARogueHealthPickup::ARogueHealthPickup()
{
	// Set collision profile, make this in Project Settings, "Overlap" only to Pawn
	OverlapComponent->SetCollisionProfileName("Pickups");

	PickupMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMeshComp"));
	PickupMeshComponent->SetCollisionProfileName("NoCollision");
	PickupMeshComponent->SetupAttachment(RootComponent);
}

void ARogueHealthPickup::OnActorOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	URogueActionSystemComponent* ActionComp = OtherActor->GetComponentByClass<URogueActionSystemComponent>();

	// Assert if null, then we misconfigured what we can overlap with, any Pawn should have an action component
	// Skip pickup if already full health
	if (ensure(ActionComp != nullptr) && !ActionComp->IsFullHealth())
	{
		// will clamp to the HealthMax
		ActionComp->ApplyHealthChange(HealingAmount);

		// Play before destroying actor, to have valid context and location
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation(), FRotator::ZeroRotator);

		// Remove Actor from world, eventually memory will be freed (garbage collection)
		Destroy();
	}
}
