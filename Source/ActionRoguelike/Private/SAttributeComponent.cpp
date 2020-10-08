// Fill out your copyright notice in the Description page of Project Settings.


#include "SAttributeComponent.h"


USAttributeComponent::USAttributeComponent()
{
	HealthMax = 100;
	Health = HealthMax;
}


bool USAttributeComponent::IsAlive() const
{
	return Health > 0.0f;
}


bool USAttributeComponent::IsFullHealth() const
{
	return Health == HealthMax;
}


float USAttributeComponent::GetHealthMax() const
{
	return HealthMax;
}


bool USAttributeComponent::ApplyHealthChange(float Delta)
{
	float OldHealth = Health;

	Health = FMath::Clamp(Health + Delta, 0.0f, HealthMax);

	float ActualDelta = Health - OldHealth;
	OnHealthChanged.Broadcast(nullptr, this, Health, ActualDelta); // @fixme: Still nullptr for InstigatorActor parameter

	return ActualDelta != 0;
}
