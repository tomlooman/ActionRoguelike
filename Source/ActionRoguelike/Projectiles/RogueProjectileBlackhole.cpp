// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectileBlackhole.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"



ARogueProjectileBlackhole::ARogueProjectileBlackhole()
{
	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComponent"));
	RadialForceComponent->SetupAttachment(RootComponent);
	RadialForceComponent->ForceStrength = -800000.0f; // Negative to pull in instead of push out
	RadialForceComponent->Radius = 1200.0f;
	// Avoid sucking player into the blackhole
	RadialForceComponent->RemoveObjectTypeToAffect(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// suck up nearby objects, small enough to let them miss the sphere and flail around a bit first
	SphereComponent->SetSphereRadius(20.0f);
	// Profile to only overlap things like physics actors and never block on anything to pass through the world
	// OnActorHit from base class will therefor never trigger as intended for this projectile
	SphereComponent->SetCollisionProfileName("BlackholeCore");
	// Slow
	ProjectileMovementComponent->InitialSpeed = 500.0f;
	
	InitialLifeSpan = 5.0f;
}

void ARogueProjectileBlackhole::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Note: Make sure GenerateOverlapEvents is enabled on the cubes in the world
	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ARogueProjectileBlackhole::OnSphereOverlappedActor);
}

void ARogueProjectileBlackhole::OnSphereOverlappedActor(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherComp->IsSimulatingPhysics())
	{
		OtherActor->Destroy();
	}
}
