// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAction.h"

#include "ActionRoguelike.h"
#include "RogueActionSystemComponent.h"
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#include "Net/UnrealNetwork.h"


void URogueAction::StartAction_Implementation()
{
	URogueActionSystemComponent* OwningComp = GetOwningComponent();
	
	OwningComp->AppendActiveTags(GrantTags);

	// Consume required resources
	for (TPair<FGameplayTag, float> Cost : ActivationCost)
	{
		OwningComp->ApplyAttributeChange(Cost.Key, -Cost.Value, Modifier);
	}
	
	bIsRunning = true;
	
	OwningComp->MulticastPlayBurstCosmetics(this);
	
	float GameTime = GetWorld()->TimeSeconds;
	UE_LOGFMT(LogGame, Log, "Started Action {ActionName} - {WorldTime}",
		("ActionName", ActionName.ToString()),
		("WorldTime", GameTime));
}

void URogueAction::StopAction_Implementation()
{
	bIsRunning = false;
	
	float GameTime = GetWorld()->TimeSeconds;
	
	UE_LOGFMT(LogGame, Log, "Stopped Action {ActionName} - {WorldTime}",
		("ActionName", ActionName.ToString()),
		("WorldTime", GameTime));

	CooldownUntil = GetWorld()->TimeSeconds + CooldownTime;

	GetOwningComponent()->RemoveActiveTags(GrantTags);
}

bool URogueAction::CanStart() const
{
	if (IsRunning())
	{
		return false;
	}
	
	if (GetCooldownTimeRemaining() > 0.0f)
	{
		UE_LOG(LogGame, Log, TEXT("Cooldown remaining: %f"), GetCooldownTimeRemaining());
		return false;
	}

	URogueActionSystemComponent* OwningComp = GetOwningComponent();
	if (OwningComp->GetActiveTags().HasAny(BlockedTags))
	{
		return false;
	}

	for (TPair<FGameplayTag, float> Cost : ActivationCost)
	{
		float AvailableAttributeAmount = OwningComp->GetAttributeValue(Cost.Key);
		if (AvailableAttributeAmount < Cost.Value)
		{
			// Not enough resources
			UE_LOGFMT(LogGame, Log, "Not enough {AttributeName} to activate {ActionName}. "
						   "Have {AvailableAttributeValue} and need {RequiredAttributeValue}",
						   ("AttributeName", Cost.Key.ToString()),
						   ("ActionName", ActionName.ToString()),
						   ("AvailableAttributeValue",AvailableAttributeAmount),
						   ("RequiredAttributeValue", Cost.Value));
			return false;
		}
	}

	return true;
}

float URogueAction::GetCooldownTimeRemaining() const
{
	return FMath::Max(0.0f, CooldownUntil - GetWorld()->TimeSeconds);
}

void URogueAction::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context,	UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	UE::Net::FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);	
}

void URogueAction::PlayBurstCosmetics_Implementation() {}

void URogueAction::OnRep_IsRunning()
{
	UE_LOG(LogGame, Log, TEXT("Received bIsRunning with OnRep_IsRunning for %s"), *ToString(GetWorld()->GetNetMode()))
}

URogueActionSystemComponent* URogueAction::GetOwningComponent() const
{
	return Cast<URogueActionSystemComponent>(GetOuter());
}

void URogueAction::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URogueAction, bIsRunning);
}