// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectileMagic.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"


ARogueProjectileMagic::ARogueProjectileMagic()
{
	ProjectileMovementComponent->InitialSpeed = 2000.0f;
}

void ARogueProjectileMagic::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Keep the base implementation
	Super::OnActorHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	FVector HitFromDirection = GetActorRotation().Vector();
	
	UGameplayStatics::ApplyPointDamage(OtherActor, 10.f, HitFromDirection, Hit,  GetInstigatorController(),
		this, DmgTypeClass);
}

