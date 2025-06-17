// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectile_Magic.h"
#include "Components/SphereComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Projectiles/RogueProjectileMovementComponent.h"
#include "ActionSystem/RogueActionEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueProjectile_Magic)


ARogueProjectile_Magic::ARogueProjectile_Magic()
{
	SphereComp->SetSphereRadius(20.0f);
	InitialLifeSpan = 10.0f;
	DamageCoefficient = 100.0f;
}


void ARogueProjectile_Magic::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// More consistent to bind here compared to Constructor which may fail to bind if Blueprint was created before adding this binding (or when using hotreload)
	// PostInitializeComponent is the preferred way of binding any events.
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ARogueProjectile_Magic::OnActorOverlap);
}


void ARogueProjectile_Magic::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		// Parry Ability (GameplayTag Example)
		URogueActionComponent* OtherActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(OtherActor);
		if (OtherActionComp && OtherActionComp->ActiveGameplayTags.HasTag(ParryTag))
		{
			MoveComp->Velocity = -MoveComp->Velocity;

			// The reflector now becomes the 'instigator' of the damage from the reflected projectile
			SetInstigator(Cast<APawn>(OtherActor));
			return;
		}
		
		// Apply Damage & Impulse
		if (URogueGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageCoefficient, SweepResult))
		{
			// We only explode if the target can be damaged, it ignores anything it Overlaps that it cannot Damage
			Explode();

			if (OtherActionComp && BurningActionClass && HasAuthority())
			{
				OtherActionComp->AddAction(GetInstigator(), BurningActionClass);
			}
		}
	}
}