// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAnimInstance.h"

#include "ActionRoguelike.h"
#include "GameplayTagContainer.h"
#include "ActionSystem/RogueActionComponent.h"
#include "SharedGameplayTags.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAnimInstance)



void URogueAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	AActor* OwningActor = GetOwningActor();
	check(OwningActor);

	ActionComp = URogueGameplayFunctionLibrary::GetActionComponentFromActor(OwningActor);
}



void URogueAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (ActionComp)
	{
		bIsStunned = ActionComp->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_Stunned);
	}
}

bool URogueAnimInstance::HandleNotify(const FAnimNotifyEvent& AnimNotifyEvent)
{
	if (AnimNotifyEvent.NotifyName == Animation::NAME_Foot_Plant_R || AnimNotifyEvent.NotifyName == Animation::NAME_Foot_Plant_L)
	{
		// Foot SFX (non-attached sounds will automatically cull if out of audible range)
		// You could choose to route this to the character classes instead to handle SFX playback
		UGameplayStatics::PlaySoundAtLocation(this, FootstepSound, GetOwningComponent()->GetComponentLocation());

		//UE_LOG(LogTemp, Log, TEXT("foot planted!"));
		return true;
	}

	// let the regular code path handle the notify
	return false;
}
