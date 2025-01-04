// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionEffect.h"
#include "ActionSystem/RogueActionComponent.h"
#include "GameFramework/GameStateBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueActionEffect)




URogueActionEffect::URogueActionEffect()
{
	bAutoStart = true;
}


void URogueActionEffect::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	if (Duration > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &ThisClass::StopAction, Instigator);

		GetWorld()->GetTimerManager().SetTimer(DurationHandle, Delegate, Duration, false);
	}

	if (Period > 0.0f)
	{
		// Can bind in-line using FTimerDelegate::CreateUObject instead
		GetWorld()->GetTimerManager().SetTimer(
			PeriodHandle,
			FTimerDelegate::CreateUObject(this, &ThisClass::ExecutePeriodicEffect, Instigator),
			Period,
			true);
	}
}


void URogueActionEffect::StopAction_Implementation(AActor* Instigator)
{
	Super::StopAction_Implementation(Instigator);

	if (GetWorld()->GetTimerManager().GetTimerRemaining(PeriodHandle) < KINDA_SMALL_NUMBER)
	{
		ExecutePeriodicEffect(Instigator);
	}

	GetWorld()->GetTimerManager().ClearTimer(PeriodHandle);
	GetWorld()->GetTimerManager().ClearTimer(DurationHandle);

	URogueActionComponent* Comp = GetOwningComponent();
	Comp->RemoveAction(this);
}


float URogueActionEffect::GetTimeRemaining() const
{
	// Possibly nullptr early on if joining as a client in multiplayer (server spawns GameState and replicates the actor instance to clients)
	if (AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>())
	{
		float EndTime = TimeStarted + Duration;
		return EndTime - GS->GetServerWorldTimeSeconds();
	}

	return Duration;
}


void URogueActionEffect::ExecutePeriodicEffect_Implementation(AActor* Instigator) {}
