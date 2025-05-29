// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionSystem/RogueAction.h"
#include "ActionSystem/RogueActionComponent.h"
#include "ActionRoguelike.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAction)


void URogueAction::Initialize(URogueActionComponent* NewActionComp)
{
	ActionComp = NewActionComp;
}


bool URogueAction::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		return false;
	}

	URogueActionComponent* Comp = GetOwningComponent();
	
	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}


void URogueAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOGFMT(LogGame, Log, "Started: {ActionName}", GetName());

	URogueActionComponent* Comp = GetOwningComponent();	
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	RepData.bIsRunning = true;
	RepData.Instigator = Instigator;

	if (GetOwningComponent()->GetOwnerRole() == ROLE_Authority)
	{
		TimeStarted = GetWorld()->TimeSeconds; 
	}

	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}


void URogueAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOGFMT(LogGame, Log, "Stopped: {name}", GetName());

	//ensureAlways(bIsRunning);

	URogueActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.RemoveTags(GrantsTags);

	RepData.bIsRunning = false;
	RepData.Instigator = Instigator;

	GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
}


UWorld* URogueAction::GetWorld() const
{
	// Outer is set when creating action via NewObject<T>
	AActor* Actor = Cast<AActor>(GetOuter());
	if (Actor)
	{
		return Actor->GetWorld();
	}

	return nullptr;
}


TSoftObjectPtr<UTexture2D> URogueAction::GetIcon() const
{
	return Icon;
}


URogueActionComponent* URogueAction::GetOwningComponent() const
{
	return ActionComp;
}


void URogueAction::OnRep_RepData()
{
	if (RepData.bIsRunning)
	{
		StartAction(RepData.Instigator);
	}
	else
	{
		StopAction(RepData.Instigator);
	}
}


bool URogueAction::IsRunning() const
{
	return RepData.bIsRunning;
}


void URogueAction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URogueAction, RepData);
	DOREPLIFETIME(URogueAction, TimeStarted);
	DOREPLIFETIME(URogueAction, ActionComp);
}