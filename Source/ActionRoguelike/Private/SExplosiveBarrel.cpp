// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "ActionRoguelike.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SExplosiveBarrel)


ASExplosiveBarrel::ASExplosiveBarrel()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	// Enabling Simulate physics automatically changes the Profile to PhysicsActor in Blueprint, in C++ we need to change this manually.
	MeshComp->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName); 
	RootComponent = MeshComp;

	ForceComp = CreateDefaultSubobject<URadialForceComponent>(TEXT("ForceComp"));
	ForceComp->SetupAttachment(MeshComp);

	// Leaving this on applies small constant force via component 'tick' (Optional)
	ForceComp->SetAutoActivate(false);

	ForceComp->Radius = 750.0f;
	ForceComp->ImpulseStrength = 2500.0f; // Alternative: 200000.0 if bImpulseVelChange = false
	// Optional, ignores 'Mass' of other objects (if false, the impulse strength will be much higher to push most objects depending on Mass)
	ForceComp->bImpulseVelChange = true;

	// Optional, default constructor of component already adds 4 object types to affect, excluding WorldDynamic
	ForceComp->AddCollisionChannelToAffect(ECC_WorldDynamic);

	ExplosionComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplosionComp"));
	ExplosionComp->bAutoActivate = false;
	ExplosionComp->SetupAttachment(MeshComp);

	FlamesFXComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FlamesFXComp"));
	FlamesFXComp->bAutoActivate = false;
	FlamesFXComp->SetupAttachment(MeshComp);

	ExplosionDelayTime = 2.0f;
}


float ASExplosiveBarrel::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	if (bExploded)
	{
		return 0.0f;
	}
	
	// Either use a hit counter or 'hitpoints' (For early assignment, we don't have any kind of attributes yet)
	HitCounter++;

	if (HitCounter == 1)
	{
		GetWorldTimerManager().SetTimer(DelayedExplosionHandle, this, &ThisClass::Explode, ExplosionDelayTime);
	}
	else if (HitCounter == 2)
	{
		// Avoid second explosion later
		GetWorldTimerManager().ClearTimer(DelayedExplosionHandle);
		
		Explode();
	}
	
	// Structured Logging Example
	UE_LOGFMT(LogGame, Log, "OnActorHit in Explosive Barrel");
	// Warnings as structured logs even show up in the "Message Log" window of UnrealEd
	UE_LOGFMT(LogGame, Warning, "OnActorHit, OtherActor: {name}, at game time: {timeseconds}", GetNameSafe(DamageCauser), GetWorld()->TimeSeconds);

	return DamageAmount;
}

void ASExplosiveBarrel::Explode()
{
	if (bExploded)
	{
		// Nothing to do here
		return;
	}

	bExploded = true;
	
	ForceComp->FireImpulse();

	ExplosionComp->Activate();

	// @todo: cause damage to other stuff around it

	//FString CombinedString = FString::Printf(TEXT("Hit at location: %s"), *Hit.ImpactPoint.ToString());
	//DrawDebugString(GetWorld(), Hit.ImpactPoint, CombinedString, nullptr, FColor::Green, 2.0f, true);
}
