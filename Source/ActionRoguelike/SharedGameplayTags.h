// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NativeGameplayTags.h"

namespace SharedGameplayTags
{
	// Actions
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Sprint);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_PrimaryAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Blackhole);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Dash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Stunned);

	// Attributes
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_HealthMax);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_AttackDamage);
}