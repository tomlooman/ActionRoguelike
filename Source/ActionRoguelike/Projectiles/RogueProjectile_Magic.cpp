// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectile_Magic.h"
#include "Components/SphereComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Projectiles/RogueProjectileMovementComponent.h"
#include "ActionSystem/RogueActionEffect.h"
#include "ActionSystem/RogueAttributeComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueProjectile_Magic)

// NOTE: With SparseDataClass feature in use, some properties are replaced with "GetXXX()" which is generated automatically by UHT.
// Example: DamageAmount becomes GetDamageAmount() without this function visible in our own header.

ARogueProjectile_Magic::ARogueProjectile_Magic()
{
	SphereComp->SetSphereRadius(20.0f);
	InitialLifeSpan = 10.0f;
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
		URogueActionComponent* OtherActionComp = OtherActor->FindComponentByClass<URogueActionComponent>();
		if (OtherActionComp && OtherActionComp->ActiveGameplayTags.HasTag(GetParryTag()))
		{
			MoveComp->Velocity = -MoveComp->Velocity;

			// The reflector now becomes the 'instigator' of the damage from the reflected projectile
			SetInstigator(Cast<APawn>(OtherActor));
			return;
		}

		// re-use the dmg amount as a 'coefficient', a percentage based off the base damage from player.
		float DmgCoefficient = GetDamageAmount();

		URogueAttributeComponent* InstigatorAttributes = URogueAttributeComponent::GetAttributes(GetInstigator());
		float TotalDamage = InstigatorAttributes->BaseDamage * (DmgCoefficient*0.01f);

		// Apply Damage & Impulse
		if (URogueGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, TotalDamage, SweepResult))
		{
			// We only explode if the target can be damaged, it ignores anything it Overlaps that it cannot Damage (it requires an AttributeComponent on the target)
			Explode();

			if (OtherActionComp && GetBurningActionClass() && HasAuthority())
			{
				OtherActionComp->AddAction(GetInstigator(), GetBurningActionClass());
			}
		}
	}
}


#if WITH_EDITOR
// Only required to convert existing properties already stored in Blueprints into the 'new' system
void ARogueProjectile_Magic::MoveDataToSparseClassDataStruct() const
{
	// make sure we don't overwrite the sparse data if it has been saved already
	const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass());
	if (BPClass == nullptr || BPClass->bIsSparseClassDataSerializable == true)
	{
		return;
	}
	
	Super::MoveDataToSparseClassDataStruct();

#if WITH_EDITORONLY_DATA
	// Unreal Header Tool (UHT) will create GetMySparseClassData automatically.
	FMagicProjectileSparseData* SparseClassData = GetMagicProjectileSparseData();

	// Modify these lines to include all Sparse Class Data properties.
	SparseClassData->DamageAmount = DamageAmount_DEPRECATED;
	SparseClassData->ParryTag = ParryTag_DEPRECATED;
	SparseClassData->BurningActionClass = BurningActionClass_DEPRECATED;
#endif // WITH_EDITORONLY_DATA
}
#endif
