// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RogueGameplayFunctionLibrary.h"

#include "ActionRoguelike.h"
#include "ShaderPipelineCache.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "ActionSystem/RogueActionSystemInterface.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueGameplayFunctionLibrary)


URogueActionComponent* URogueGameplayFunctionLibrary::GetActionComponentFromActor(AActor* FromActor)
{
	// Note: Cast<T> on interface only works if the interface was implemented on the Actor in C++
	// For BP implemented we should change this code to call Execute_GetActionComponent instead...
	const IRogueActionSystemInterface* ASI = Cast<IRogueActionSystemInterface>(FromActor);
	if (ASI)
	{
		return ASI->GetActionComponent();
	}
	/*if (InActor && InActor->Implements<URogueGameplayInterface>()) // example reference for a BP interface
	{
		URogueActionComponent* ActionComp = nullptr;
		if (IRogueGameplayInterface::Execute_GetActionComponent(InActor, ActionComp))
		{
			return ActionComp;
		}
	}*/

	// Fallback when interface is missing
	return FromActor->FindComponentByClass<URogueActionComponent>();
}

bool URogueGameplayFunctionLibrary::IsAlive(AActor* InActor)
{
	if (!IsValid(InActor))
	{
		return false;
	}

	URogueActionComponent* ActionComp = GetActionComponentFromActor(InActor);
	if (ActionComp)
	{
		return ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health)->GetValue() > 0.0f;
	}

	return false;
}


bool URogueGameplayFunctionLibrary::KillActor(AActor* InActor)
{
	URogueActionComponent* ActionComp = URogueActionComponent::GetActionComponent(InActor);
	const FRogueAttribute* HealthMaxAttribute = ActionComp->GetAttribute(SharedGameplayTags::Attribute_HealthMax);
	
	return ActionComp->ApplyAttributeChange(SharedGameplayTags::Attribute_Health, HealthMaxAttribute->GetValue(),
		InActor, EAttributeModifyType::AddBase);
}


bool URogueGameplayFunctionLibrary::IsFullHealth(AActor* InActor)
{
	URogueActionComponent* ActionComp = URogueActionComponent::GetActionComponent(InActor);

	const FRogueAttribute* HealthAttribute = ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health);
	const FRogueAttribute* HealthMaxAttribute = ActionComp->GetAttribute(SharedGameplayTags::Attribute_HealthMax);

	return HealthAttribute->GetValue() >= HealthMaxAttribute->GetValue();
}


bool URogueGameplayFunctionLibrary::ApplyDamage(AActor* DamageCauser, AActor* TargetActor, float DamageCoefficient, FGameplayTagContainer InContextTags)
{
	if (!TargetActor->CanBeDamaged())
	{
		// Support things like godmode for player
		return false;
	}
	
	URogueActionComponent* InstigatorComp = GetActionComponentFromActor(DamageCauser);
	check(InstigatorComp);

	FRogueAttribute* FoundAttribute = InstigatorComp->GetAttribute(SharedGameplayTags::Attribute_AttackDamage);

	// We might not have implemented the new attributes on every actor yet.
	// @fixme: Assert for now, later we just log this as we might not want to deal dmg to some things.
	check(FoundAttribute);

	// Coefficient is a %, to scale all out damage off the instigator's base attack damage
	float TotalDamage = FoundAttribute->GetValue() * (DamageCoefficient*0.01f);

	URogueActionComponent* VictimComp = GetActionComponentFromActor(TargetActor);
	if (VictimComp == nullptr)
	{
		UE_LOG(LogGame, Warning, TEXT("ApplyDamage Victim (%s) does not contain an ActionComponent."), *GetNameSafe(TargetActor));
		return false;
	}

	FAttributeModification AttriMod = FAttributeModification(
		SharedGameplayTags::Attribute_Health,
		-TotalDamage, // Make sure we apply a negative amount to the Health
		VictimComp,
		DamageCauser,
		EAttributeModifyType::AddBase,
		InContextTags);

	// Could pass through dead enemies
	return VictimComp->ApplyAttributeChange(AttriMod);
}


bool URogueGameplayFunctionLibrary::ApplyDirectionalDamage(AActor* DamageCauser, AActor* TargetActor, float DamageCoefficient, const FHitResult& HitResult, FGameplayTagContainer InContextTags)
{
	if (ApplyDamage(DamageCauser, TargetActor, DamageCoefficient, InContextTags))
	{
		UPrimitiveComponent* HitComp = HitResult.GetComponent();
		if (HitComp->bApplyImpulseOnDamage && HitComp->IsSimulatingPhysics(HitResult.BoneName))
		{
			// Direction = Target - Origin
			FVector Direction = HitResult.TraceEnd - HitResult.TraceStart;
			Direction.Normalize();

			// @todo: allow configuration for impulse strength
			HitComp->AddImpulseAtLocation(Direction * 30000.f, HitResult.ImpactPoint, HitResult.BoneName);
		}
		return true;
	}


	// Call into Unreal built in logic for early course damaging of explosive barrel
	// Don't have a proper way of knowing it was processed, so we return false
	//UGameplayStatics::ApplyDamage(TargetActor, DamageAmount, nullptr, DamageCauser, nullptr);

	return false;
}

/*
bool URogueGameplayFunctionLibrary::ApplyRadialDamage(AActor* DamageCauser, FVector Origin, float DamageRadius, float DamageCoefficient)
{
	UWorld* World = DamageCauser->GetWorld();
	// do async overlap to find list of potential victims
	// only test for actors with action component / or gameobject interface
	// 2nd pass is another async trace for occlusion tests (optional)
	// GameObject interface: GetDamageTraceLocations(TArray<FVector>& OutLocations);
	// allow objects or pawns to specify which locations they want to use for occlusion tests, for example
	// head, spine, hands, legs on a character
	// for large and oddly shapes objects, it can also use "nearest collision point" from origin rather than the actor location

	FCollisionShape Shape;
	Shape.SetSphere(DamageRadius);

	FCollisionQueryParams Params;
	//Params.MobilityType = EQueryMobilityType::Dynamic;

	FCollisionResponseParams ResponseParams;
*/
	/*
	FOverlapDelegate* Delegate;
	Delegate->BindLambda([](const FTraceHandle& Handle, FOverlapDatum& Datum)
		{
			// ... called when ready
		});*/

	//FTraceHandle Handle; // @todo: can pass in additional params here if needed for multi-pass stuff
	// @todo:need to pass "this", which wont work in static function
/*
	// Fill any useful dmg info
	FDamageInfo Info;
	Info.DamageInstigator = DamageCauser;
	Info.AttackDamage = 0.0f; // InstigatorDmg * (DamageCoefficient*0.01f)
	
	FOverlapDelegate Delegate = FOverlapDelegate::CreateUObject(this, &URogueGameplayFunctionLibrary::OnDamageOverlapComplete, Info);

	World->AsyncOverlapByChannel(Origin, FQuat::Identity, COLLISION_PROJECTILE,
		Shape, Params, ResponseParams, &Delegate);



	return false;
}
*/

/*	
void URogueGameplayFunctionLibrary::OnDamageOverlapComplete(const FTraceHandle& TraceHandle, FOverlapDatum& OverlapDatum, FDamageInfo DamageInfo)
{
	// if second pass w/ line traces is async too, we are two frames 'behind' the initial request for damage.

	// @todo: iterate the victims to apply damage: OverlapDatum.OutOverlaps

	check(DamageInfo.DamageInstigator.Get());

	for (FOverlapResult& Overlap : OverlapDatum.OutOverlaps)
	{
		ApplyDamage(DamageInfo.DamageInstigator.Get(), Overlap.GetActor(), DamageInfo.AttackDamage);
	}
}
*/

int32 URogueGameplayFunctionLibrary::GetRemainingBundledPSOs()
{
	// Counts Bundled PSOs remaining, exposed for UI access
	return FShaderPipelineCache::NumPrecompilesRemaining();
}
