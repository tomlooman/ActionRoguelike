// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionEffect_StunMonster.h"
#include "RogueActionSystemComponent.h"
#include "SharedGameplayTags.h"



URogueActionEffect_StunMonster::URogueActionEffect_StunMonster()
{
	// Matches available animation duration for RangedMinion
	Duration = 2.5f;
	GrantTags.AddTag(SharedGameplayTags::StatusEffect_Stunned);
}