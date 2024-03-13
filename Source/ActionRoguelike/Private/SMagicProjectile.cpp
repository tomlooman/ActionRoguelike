// Fill out your copyright notice in the Description page of Project Settings.


#include "SMagicProjectile.h"
#include "Components/SphereComponent.h"
#include "SGameplayFunctionLibrary.h"
#include "SActionComponent.h"
#include "Components/SProjectileMovementComponent.h"
#include "SActionEffect.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SMagicProjectile)

// NOTE: With SparseDataClass feature in use, some properties are replaced with "GetXXX()" which is generated automatically by UHT.
// Example: DamageAmount becomes GetDamageAmount() without this function visible in our own header.

ASMagicProjectile::ASMagicProjectile()
{
	SphereComp->SetSphereRadius(20.0f);
	InitialLifeSpan = 10.0f;
}


void ASMagicProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// More consistent to bind here compared to Constructor which may fail to bind if Blueprint was created before adding this binding (or when using hotreload)
	// PostInitializeComponent is the preferred way of binding any events.
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ASMagicProjectile::OnActorOverlap);
}


void ASMagicProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		// Parry Ability (GameplayTag Example)
		USActionComponent* ActionComp = OtherActor->FindComponentByClass<USActionComponent>();
		if (ActionComp && ActionComp->ActiveGameplayTags.HasTag(GetParryTag()))
		{
			MoveComp->Velocity = -MoveComp->Velocity;

			SetInstigator(Cast<APawn>(OtherActor));
			return;
		}

		// Apply Damage & Impulse
		if (USGameplayFunctionLibrary::ApplyDirectionalDamage(GetInstigator(), OtherActor, GetDamageAmount(), SweepResult))
		{
			// We only explode if the target can be damaged, it ignores anything it Overlaps that it cannot Damage (it requires an AttributeComponent on the target)
			Explode();

			if (ActionComp && GetBurningActionClass() && HasAuthority())
			{
				ActionComp->AddAction(GetInstigator(), GetBurningActionClass());
			}
		}
	}
}


#if WITH_EDITOR
// Only required to convert existing properties already stored in Blueprints into the 'new' system
void ASMagicProjectile::MoveDataToSparseClassDataStruct() const
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
