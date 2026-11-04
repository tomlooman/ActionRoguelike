// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAttributeSet.h"

#include "RogueActionComponent.h"
#include "SharedGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"


URogueMonsterAttributeSet::URogueMonsterAttributeSet()
{
	// Override with lower default attack damage than players
	AttackDamage = FRogueAttribute(10);
		
	MoveSpeed = FRogueAttribute(450);
}

// -- Health Attribute Set -- //

void URogueHealthAttributeSet::OnRep_Health(FRogueAttribute OldValue)
{
	float NewValue = Health.GetValue();

	FAttributeModification Modification;
	Modification.AttributeTag = SharedGameplayTags::Attribute_Health;
	Modification.Magnitude = Health.GetValue() - OldValue.GetValue();
	Modification.TargetComp = GetOwningComponent();

	GetOwningComponent()->BroadcastAttributeChanged(SharedGameplayTags::Attribute_Health, NewValue, Modification);
}


void URogueAttributeSet::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}


URogueActionComponent* URogueAttributeSet::GetOwningComponent() const
{
	return Cast<URogueActionComponent>(GetOuter());
}


void URogueHealthAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URogueHealthAttributeSet, Health);
}

// -- Survivor Attribute Set -- //

void URogueSurvivorAttributeSet::PostAttributeChanged()
{
	Super::PostAttributeChanged();

	const float MaxRage = RageMax.GetValue();
	Rage.Base = FMath::Clamp(Rage.Base, 0.0f, MaxRage);
	// Gains and costs use AddModifier. Discard overflow so spending at the cap takes effect immediately.
	Rage.Modifier = FMath::Clamp(Rage.Modifier, -Rage.Base, MaxRage - Rage.Base);
}


void URogueSurvivorAttributeSet::OnRep_Rage(FRogueAttribute OldValue)
{
	float NewValue = Rage.GetValue();

	FAttributeModification Modification;
	Modification.AttributeTag = SharedGameplayTags::Attribute_Rage;
	Modification.Magnitude = NewValue - OldValue.GetValue();
	Modification.TargetComp = GetOwningComponent();

	GetOwningComponent()->BroadcastAttributeChanged(SharedGameplayTags::Attribute_Rage, NewValue, Modification);
}


void URogueSurvivorAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(URogueSurvivorAttributeSet, Rage, COND_OwnerOnly);
}

// -- Pawn Attribute Set -- //

void URoguePawnAttributeSet::OnRep_MoveSpeed()
{
	ApplyMovementSpeed();
}


void URoguePawnAttributeSet::ApplyMovementSpeed()
{
	// Assume all Pawns are Characters with CMC (may change with Mover 2.0)
	ACharacter* OwningCharacter = CastChecked<ACharacter>(GetOwningComponent()->GetOwner());
	OwningCharacter->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed.GetValue();
}


void URoguePawnAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URoguePawnAttributeSet, MoveSpeed);
}
