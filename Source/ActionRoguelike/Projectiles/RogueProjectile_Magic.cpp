// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectile_Magic.h"
#include "Components/SphereComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Projectiles/RogueProjectileMovementComponent.h"
#include "ActionSystem/RogueActionEffect.h"
#include "Core/RogueDeferredTaskSystem.h"

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

		FGameplayTagContainer ContextTags;

		// @todo: check CanApplyDamage() so we know on the clients to explode rather than passthrough the Overlap.
		// this keeps them in sync even if the server will notify the client a bit later from latency
		
		// Apply Damage & Impulse
		if (URogueGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, DamageCoefficient, SweepResult, ContextTags))
		{
			// We only explode if the target can be damaged, it ignores anything it Overlaps that it cannot Damage
			Explode();
			
			APawn* MyInstigator = GetInstigator();
			if (OtherActionComp && BurningActionClass && HasAuthority())
			{
#if USE_DEFERRED_TASKS
				// Delay adding the burning FX if necessary
				URogueDeferredTaskSystem::AddTask(this, [OtherActionComp,MyInstigator,this]()
					{
						OtherActionComp->AddAction(MyInstigator, BurningActionClass);
					});
#else
				OtherActionComp->AddAction(MyInstigator, BurningActionClass);
#endif
				
			}
		}
	}
}