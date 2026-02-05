// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAttributeSet.h"

URogueHealthAttributeSet::URogueHealthAttributeSet()
{
	Health = FRogueAttribute(100);
	HealthMax = FRogueAttribute(Health.GetValue());
}

void URogueHealthAttributeSet::PostAttributeChanged()
{
	Health.Base = FMath::Clamp(Health.Base, 0.0f, HealthMax.GetValue());
}
