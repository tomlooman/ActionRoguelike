// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueExplosiveBarrel.h"
#include "ActionRoguelike.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "NiagaraComponent.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueExplosiveBarrel)


ARogueExplosiveBarrel::ARogueExplosiveBarrel()
{
	ActionComp = CreateDefaultSubobject<URogueActionComponent>(TEXT("ActionComp"));
	ActionComp->SetDefaultAttributeSet(URogueHealthAttributeSet::StaticClass());
	
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

void ARogueExplosiveBarrel::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FAttributeChangedSignature& Delegate = ActionComp->GetAttributeListenerDelegate(SharedGameplayTags::Attribute_Health);
	Delegate.AddUObject(this, &ThisClass::OnHealthAttributeChanged);
}


void ARogueExplosiveBarrel::OnHealthAttributeChanged(float NewValue, const FAttributeModification& AttributeModification)
{
	if (bExploded)
	{
		return;
	}
	
	// Either use a hit counter or 'hitpoints' (For early assignment, we don't have any kind of attributes yet)
	HitCounter++;

	if (HitCounter == 1)
	{
		GetWorldTimerManager().SetTimer(DelayedExplosionHandle, this, &ThisClass::Explode, ExplosionDelayTime);

		FlamesFXComp->Activate();
	}
	else if (HitCounter == 2)
	{
		// Avoid second explosion later
		GetWorldTimerManager().ClearTimer(DelayedExplosionHandle);
		
		Explode();
	}
	
	UE_LOG(LogGame, Log, TEXT("OnActorHit in Explosive Barrel"));
	// Structured Logging Example
	UE_LOGFMT(LogGame, Warning, "OnActorHit, OtherActor: {name}, at game time: {timeseconds}",
		("name", GetNameSafe(AttributeModification.Instigator->GetOwner())),
		("timeseconds", GetWorld()->TimeSeconds));
}


void ARogueExplosiveBarrel::Explode()
{
	if (bExploded)
	{
		// Nothing to do here
		return;
	}

	bExploded = true;

	FlamesFXComp->Deactivate();
	
	ForceComp->FireImpulse();

	ExplosionComp->Activate();

	// @todo: cause damage to other stuff around it

	//FString CombinedString = FString::Printf(TEXT("Hit at location: %s"), *Hit.ImpactPoint.ToString());
	//DrawDebugString(GetWorld(), Hit.ImpactPoint, CombinedString, nullptr, FColor::Green, 2.0f, true);
}

