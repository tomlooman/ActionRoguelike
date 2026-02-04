// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAttributeSet.h"

URogueHealthAttributeSet::URogueHealthAttributeSet()
{
	Health = FRogueAttribute(100);
	HealthMax = FRogueAttribute(Health.GetValue());
}
