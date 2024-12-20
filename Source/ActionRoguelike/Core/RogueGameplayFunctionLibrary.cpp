// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RogueGameplayFunctionLibrary.h"
#include "ActionSystem/RogueAttributeComponent.h"
#include "ShaderPipelineCache.h"
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


int32 URogueGameplayFunctionLibrary::GetRemainingBundledPSOs()
{
	// Counts Bundled PSOs remaining, exposed for UI access
	return FShaderPipelineCache::NumPrecompilesRemaining();
}
