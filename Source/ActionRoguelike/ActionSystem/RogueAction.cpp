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

	// Only run on server from now. (owning) clients take a different path.
	check(GetOwningComponent()->GetOwnerRole() == ROLE_Authority);
	
	// @todo: this now needs to be replicated
	URogueActionComponent* Comp = GetOwningComponent();	
	Comp->AppendActiveTags(GrantsTags);

	
	RepData.bIsRunning = true;
	RepData.Instigator = Instigator;

	TimeStarted = GetWorld()->TimeSeconds; 
	
	SetActionCosmeticsActive(true);
	
	//MulticastPlayCosmetics();
	GetOwningComponent()->MulticastPlayActionCosmetics(this);

	GetOwningComponent()->OnActionStarted.Broadcast(GetOwningComponent(), this);
}


void URogueAction::StopAction_Implementation(AActor* Instigator)
{
	UE_LOGFMT(LogGame, Verbose, "Stopped: {ActionName}", ("ActionName", GetName()));

	//ensureAlways(bIsRunning);
	check(GetOwningComponent()->GetOwnerRole() == ROLE_Authority);

	URogueActionComponent* Comp = GetOwningComponent();
	Comp->RemoveActiveTags(GrantsTags);

	RepData.bIsRunning = false;
	RepData.Instigator = Instigator;
	
	SetActionCosmeticsActive(false);
	
	if (CooldownTime > 0.0f)
	{
		CooldownUntil = GetWorld()->TimeSeconds + CooldownTime;
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


URogueActionComponent* URogueAction::GetOwningComponent() const
{
	return Cast<URogueActionComponent>(GetOuter());
}

AActor* URogueAction::GetOwningActor() const
{
	return GetOwningComponent()->GetOwner();
}


void URogueAction::OnRep_RepData()
{
	ApplyActionState(RepData.bIsRunning);
	
	SetActionCosmeticsActive(RepData.bIsRunning);
}


void URogueAction::ApplyActionState_Implementation(bool bIsRunning)
{
	// Nothing here, sprint applies state like AirControl
}


void URogueAction::PlayActionCosmetics()
{
	// Nothing to do here, projectile attack handles one-shot cosmetics
}

void URogueAction::SetActionCosmeticsActive_Implementation(bool bIsActive)
{
	// Nothing to do here, sprint handled start/stop for cosmetics
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
