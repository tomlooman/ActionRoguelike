// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAttributeSet.h"

#include "RogueActionComponent.h"
#include "SharedGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// -- Health Attribute Set -- //

void URogueHealthAttributeSet::OnRep_Health(FRogueAttribute OldValue)
{
	float NewValue = Health.GetValue();

	FAttributeModification Modification;
	Modification.AttributeTag = SharedGameplayTags::Attribute_Health;
	Modification.Magnitude = Health.GetValue() - OldValue.GetValue();
	Modification.TargetComp = OwningComp;

	// @todo: swap out modification struct with flat params.
	OwningComp->GetAttributeListenerDelegate(SharedGameplayTags::Attribute_Health).Broadcast(NewValue, Modification);
}


void URogueHealthAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URogueHealthAttributeSet, Health);
}

// -- Pawn Attribute Set -- //

void URoguePawnAttributeSet::ApplyMovementSpeed()
{
	// Assume all Pawns are Characters with CMC (may change with Mover 2.0)
	ACharacter* OwningCharacter = CastChecked<ACharacter>(OwningComp->GetOwner());
	OwningCharacter->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed.GetValue();
}
