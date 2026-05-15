// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionEffect_StunFromDamage.h"

#include "ActionRoguelike.h"
#include "RogueActionComponent.h"
#include "SharedGameplayTags.h"
#include "Logging/StructuredLog.h"



void URogueActionEffect_StunFromDamage::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);
	
	GetOwningComponent()->GetAttributeListenerDelegate(SharedGameplayTags::Attribute_Health).AddUObject(this, 
		&ThisClass::OnHealthChanged);
}


void URogueActionEffect_StunFromDamage::OnHealthChanged(float NewValue, const FAttributeModification& AttriMod)
{
	if (AttriMod.Magnitude >= 0.0f)
	{
		// ignore healing
		return;
	}
	
	const float TimeNow = GetWorld()->TimeSeconds;
	// Did less time pass than the threshold?
	const float DeltaTime = TimeNow - LastDamageTime;
	
	// Do not count until Stunned has expired (avoid stunlocking)
	if (GetOwningComponent()->GetActiveTags().HasTag(SharedGameplayTags::Status_Stunned))
	{
		return;
	}
	
	if (DeltaTime < TimeDeltaThreshold)
	{
		SummedRecentDamage += FMath::Abs(AttriMod.Magnitude);
	}
	else
	{
		SummedRecentDamage = FMath::Abs(AttriMod.Magnitude);
	}
	
	LastDamageTime = TimeNow;
	
	// Trigger stun?
	if (SummedRecentDamage >= DamageThreshold)
	{
		GetOwningComponent()->AddAction(AttriMod.Instigator.Get(), EffectOnThreshold);
		
		UE_LOG(LogGame, Log, TEXT("Stunned applied to %s"), *GetNameSafe(GetOwningComponent()->GetOwner()));
		
		//UE_LOGFMT(LogGame, Log, "Stunned Debuff applied to {owningActor}", GetNameSafe(GetOwningComponent()->GetOwner());
		
		// Reset
		SummedRecentDamage = 0.0f;
		LastDamageTime = 0.0f;
	}
}