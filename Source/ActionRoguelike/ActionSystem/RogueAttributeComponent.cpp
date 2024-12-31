// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionSystem/RogueAttributeComponent.h"
#include "Core/RogueGameModeBase.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAttributeComponent)

static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("game.DamageMultiplier"), 1.0f, TEXT("Global Damage Modifier for Attribute Component."), ECVF_Cheat);


URogueAttributeComponent::URogueAttributeComponent()
{
	HealthMax = 100;
	Health = HealthMax;

	Rage = 0;
	RageMax = 100;

	BaseDamage = 20.0f;

	SetIsReplicatedByDefault(true);
}


bool URogueAttributeComponent::ApplyHealthChange(AActor* InstigatorActor, float Delta)
{
	if (!GetOwner()->CanBeDamaged() && Delta < 0.0f)
	{
		return false;
	}

	if (Delta < 0.0f)
	{
		const float DamageMultiplier = CVarDamageMultiplier.GetValueOnGameThread();
		Delta *= DamageMultiplier;
	}

	float OldHealth = Health;
	float NewHealth = FMath::Clamp(Health + Delta, 0.0f, HealthMax);

	float ActualDelta = NewHealth - OldHealth;

	// Is Server?
	if (GetOwner()->HasAuthority())
	{
		Health = NewHealth;

		if (!FMath::IsNearlyZero(ActualDelta))
		{
			MulticastHealthChanged(InstigatorActor, Health, ActualDelta);
		}

		// Died
		if (ActualDelta < 0.0f && FMath::IsNearlyZero(Health))
		{
			ARogueGameModeBase* GM = GetWorld()->GetAuthGameMode<ARogueGameModeBase>();
			if (GM)
			{
				GM->OnActorKilled(GetOwner(), InstigatorActor);
			}
		}
	}
	
	return !FMath::IsNearlyZero(ActualDelta);
}


float URogueAttributeComponent::GetRage() const
{
	return Rage;
}


bool URogueAttributeComponent::ApplyRage(AActor* InstigatorActor, float Delta)
{
	const float OldRage = Rage;

	Rage = FMath::Clamp(Rage + Delta, 0.0f, RageMax);

	float ActualDelta = Rage - OldRage;

	// Will be zero delta if we already at max or min
	if (!FMath::IsNearlyZero(ActualDelta))
	{
		OnRageChanged.Broadcast(InstigatorActor, this, Rage, ActualDelta);
		return true;
	}

	return false;
}


URogueAttributeComponent* URogueAttributeComponent::GetAttributes(AActor* FromActor)
{
	if (FromActor)
	{
		// @todo: allow "quick access" via interface callback to optionally implement
		return FromActor->FindComponentByClass<URogueAttributeComponent>();
	}

	return nullptr;
}

/*
bool URogueAttributeComponent::IsActorAlive(AActor* Actor)
{
	URogueAttributeComponent* AttributeComp = GetAttributes(Actor);
	if (AttributeComp)
	{
		return AttributeComp->IsAlive();
	}

	return false;
}
*/

bool URogueAttributeComponent::Kill(AActor* InstigatorActor)
{
	return ApplyHealthChange(InstigatorActor, -GetHealthMax());
}


bool URogueAttributeComponent::IsAlive() const
{
	return Health > 0.0f;
}


bool URogueAttributeComponent::IsFullHealth() const
{
	return FMath::IsNearlyEqual(Health, HealthMax);
}


float URogueAttributeComponent::GetHealth() const
{
	return Health;
}


float URogueAttributeComponent::GetHealthMax() const
{
	return HealthMax;
}


void URogueAttributeComponent::MulticastHealthChanged_Implementation(AActor* InstigatorActor, float NewHealth, float Delta)
{
	OnHealthChanged.Broadcast(InstigatorActor, this, NewHealth, Delta);
}


void URogueAttributeComponent::MulticastRageChanged_Implementation(AActor* InstigatorActor, float NewRage, float Delta)
{
	OnRageChanged.Broadcast(InstigatorActor, this, NewRage, Delta);
}


void URogueAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URogueAttributeComponent, Health);
	DOREPLIFETIME(URogueAttributeComponent, HealthMax);

	DOREPLIFETIME(URogueAttributeComponent, Rage);
	DOREPLIFETIME(URogueAttributeComponent, RageMax);
}