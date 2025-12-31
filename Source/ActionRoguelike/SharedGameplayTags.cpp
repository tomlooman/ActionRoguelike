// Fill out your copyright notice in the Description page of Project Settings.


#include "SharedGameplayTags.h"

namespace SharedGameplayTags
{
	// Actions
	UE_DEFINE_GAMEPLAY_TAG(Action_Sprint, "Action.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Action_PrimaryAttack, "Action.PrimaryAttack");
	UE_DEFINE_GAMEPLAY_TAG(Action_Blackhole, "Action.Blackhole");
	UE_DEFINE_GAMEPLAY_TAG(Action_Dash, "Action.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Action_Melee, "Action.Melee");
	
	// Status
	UE_DEFINE_GAMEPLAY_TAG(Status_Stunned, "Status.Stunned");

	// Damage Context
	UE_DEFINE_GAMEPLAY_TAG(Context_Reflected, "Context.Reflected");

	// Attributes
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Health, "Attribute.Health");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_HealthMax, "Attribute.HealthMax");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_AttackDamage, "Attribute.AttackDamage");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Rage, "Attribute.Rage");
	UE_DEFINE_GAMEPLAY_TAG(Attribute_Credits, "Attribute.Credits");

	// Messaging
	UE_DEFINE_GAMEPLAY_TAG(Message_MonsterKilled, "Message.MonsterKilled");
	
	// UE_DEFINE_GAMEPLAY_TAG_COMMENT()
}
