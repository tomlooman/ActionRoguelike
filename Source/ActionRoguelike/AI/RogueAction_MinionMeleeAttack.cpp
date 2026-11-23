// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAction_MinionMeleeAttack.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Animation/RogueAnimInstance.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"


URogueAction_MinionMeleeAttack::URogueAction_MinionMeleeAttack()
{
	ActivationTag = SharedGameplayTags::Action_Melee;
}


void URogueAction_MinionMeleeAttack::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	bDamageApplied = false;

	check(IsInGameThread());
	
	ACharacter* MyPawn = CastChecked<ACharacter>(Instigator);
	URogueAnimInstance* AnimInst = Cast<URogueAnimInstance>(MyPawn->GetMesh()->GetAnimInstance());
	OnOverlapHandle = AnimInst->OnMeleeOverlap.AddUObject(this, &ThisClass::OnMeleeOverlaps);

	float Duration = MyPawn->PlayAnimMontage(MeleeMontage);

	// Stop once the animation has finished
	GetWorld()->GetTimerManager().SetTimer(AnimCompleteHandle,
		FTimerDelegate::CreateUObject(this, &ThisClass::StopAction, Instigator), Duration, false);
}

void URogueAction_MinionMeleeAttack::StopAction_Implementation(AActor* Instigator)
{
	GetWorld()->GetTimerManager().ClearTimer(AnimCompleteHandle);

	ACharacter* MyPawn = CastChecked<ACharacter>(Instigator);
	URogueAnimInstance* AnimInst = Cast<URogueAnimInstance>(MyPawn->GetMesh()->GetAnimInstance());
	AnimInst->OnMeleeOverlap.Remove(OnOverlapHandle);

	Super::StopAction_Implementation(Instigator);
}

void URogueAction_MinionMeleeAttack::OnMeleeOverlaps(const TArray<FOverlapResult>& Overlaps)
{
	if (bDamageApplied)
	{
		// Skip
		// todo: alternatively, we could still allow one damage per actor, so it can melee-hit multiple different actors
		// across multiple "triggers" of a melee animation
		return;
	}
	
	// Find "best" overlap and deal damage to it.
	AActor* BestOverlap = nullptr;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		// todo: check, if enemy, etc.
		if (Overlap.GetActor()->CanBeDamaged())
		{
			BestOverlap = Overlap.GetActor();
			break;
		}
	}

	if (BestOverlap)
	{
		URogueGameplayFunctionLibrary::ApplyDamage(GetOwningComponent()->GetOwner(), BestOverlap, DamageCoefficient);

		// Only allow damage once
		bDamageApplied = true;
	}
}
