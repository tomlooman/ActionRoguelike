// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectileMagic.h"

#include "ActionSystem/RogueActionSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ActionSystem/RogueActionEffect.h"
#include "GameFramework/ProjectileMovementComponent.h"


ARogueProjectileMagic::ARogueProjectileMagic()
{
	ProjectileMovementComponent->InitialSpeed = 2000.0f;

	InitialLifeSpan = 8.0f;
}

void ARogueProjectileMagic::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Keep the base implementation
	Super::OnActorHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	FVector HitFromDirection = GetActorRotation().Vector();
	
	UGameplayStatics::ApplyPointDamage(OtherActor, 10.f, HitFromDirection, Hit,  GetInstigatorController(),
		this, DmgTypeClass);
	
	if (EffectOnHit)
	{
		URogueActionSystemComponent* ActionComp = OtherActor->FindComponentByClass<URogueActionSystemComponent>();
		if (ActionComp) // Not everything will have one
		{
			ActionComp->GrantAction(EffectOnHit);
		}
	}
}

