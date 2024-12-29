// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RogueGameplayFunctionLibrary.h"

#include "ActionRoguelike.h"
#include "ActionSystem/RogueAttributeComponent.h"
#include "ShaderPipelineCache.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueGameplayFunctionLibrary)


bool URogueGameplayFunctionLibrary::ApplyDamage(AActor* DamageCauser, AActor* TargetActor, float DamageAmount)
{
	URogueAttributeComponent* AttributeComp = URogueAttributeComponent::GetAttributes(TargetActor);
	if (AttributeComp)
	{
		return AttributeComp->ApplyHealthChange(DamageCauser, -DamageAmount);
	}
	return false;
}


bool URogueGameplayFunctionLibrary::ApplyDirectionalDamage(AActor* DamageCauser, AActor* TargetActor, float DamageAmount, const FHitResult& HitResult)
{
	if (ApplyDamage(DamageCauser, TargetActor, DamageAmount))
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
	// Don't have a proper way of knowing it was processed
	UGameplayStatics::ApplyDamage(TargetActor, DamageAmount, nullptr, DamageCauser, nullptr);

	return false;
}

bool URogueGameplayFunctionLibrary::ApplyRadialDamage(AActor* DamageCauser, FVector Origin, float DamageRadius, float DamageAmount)
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

	/*
	FOverlapDelegate* Delegate;
	Delegate->BindLambda([](const FTraceHandle& Handle, FOverlapDatum& Datum)
		{
			// ... called when ready
		});*/

	//FTraceHandle Handle; // @todo: can pass in additional params here if needed for multi-pass stuff
	// @todo:need to pass "this", which wont work in static function

	// Fill any useful dmg info
	FDamageInfo Info;
	Info.DamageInstigator = DamageCauser;
	Info.AttackDamage = DamageAmount;
	
	FOverlapDelegate Delegate = FOverlapDelegate::CreateUObject(this, &URogueGameplayFunctionLibrary::OnDamageOverlapComplete, Info);

	World->AsyncOverlapByChannel(Origin, FQuat::Identity, COLLISION_PROJECTILE,
		Shape, Params, ResponseParams, &Delegate);



	return false;
}

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


int32 URogueGameplayFunctionLibrary::GetRemainingBundledPSOs()
{
	// Counts Bundled PSOs remaining, exposed for UI access
	return FShaderPipelineCache::NumPrecompilesRemaining();
}
