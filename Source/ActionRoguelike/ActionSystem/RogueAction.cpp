// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionSystem/RogueAction.h"
#include "ActionSystem/RogueActionComponent.h"
#include "ActionRoguelike.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAction)


bool URogueAction::CanStart_Implementation(AActor* Instigator)
{
	if (IsRunning())
	{
		return false;
	}

	if (CooldownTimeRemaining() > 0.0f)
	{
		UE_LOG(LogGame, Log, TEXT("CanStart failed, cooling down for %f seconds"), CooldownTimeRemaining());
		return false;
	}

	URogueActionComponent* Comp = GetOwningComponent();
	
	if (Comp->GetActiveTags().HasAny(BlockedTags))
	{
		return false;
	}

	return true;
}


void URogueAction::StartAction_Implementation(AActor* Instigator)
{
	UE_LOGFMT(LogGame, Verbose, "Started: {ActionName}", ("ActionName", GetName()));

	URogueActionComponent* Comp = GetOwningComponent();	
	Comp->AppendActiveTags(GrantsTags);

	if (GetOwningComponent()->GetOwnerRole() == ROLE_Authority)
	{
		RepData.bIsRunning = true;
		RepData.Instigator = Instigator;

		TimeStarted = GetWorld()->TimeSeconds; 
	}

	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}


void URogueAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOGFMT(LogGame, Verbose, "Stopped: {ActionName}", ("ActionName", GetName()));

	//ensureAlways(bIsRunning);

	URogueActionComponent* Comp = GetOwningComponent();
	Comp->RemoveActiveTags(GrantsTags);

	if (GetOwningComponent()->GetOwnerRole() == ROLE_Authority)
	{
		RepData.bIsRunning = false;
		RepData.Instigator = Instigator;
		
		if (CooldownTime > 0.0f)
		{
			CooldownUntil = GetWorld()->TimeSeconds + CooldownTime;
		}
	}

	GetOwningComponent()->OnActionStopped.Broadcast(GetOwningComponent(), this);
}


float URogueAction::CooldownTimeRemaining() const
{
	// For multiplayer, use the server time 
	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		return FMath::Max(0.0f, CooldownUntil - GS->GetServerWorldTimeSeconds());
	}
	
	return 0.0f;
}


TSoftObjectPtr<UTexture2D> URogueAction::GetIcon() const
{
	return Icon;
}


URogueActionComponent* URogueAction::GetOwningComponent() const
{
	return Cast<URogueActionComponent>(GetOuter());
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


void URogueAction::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}


void URogueAction::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URogueAction, RepData);
	DOREPLIFETIME(URogueAction, TimeStarted);
	DOREPLIFETIME_CONDITION(URogueAction, CooldownUntil, COND_OwnerOnly);
}
