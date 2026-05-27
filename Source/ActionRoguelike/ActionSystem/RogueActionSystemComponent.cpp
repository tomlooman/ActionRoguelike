// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionSystemComponent.h"

#include "ActionRoguelike.h"
#include "RogueAction.h"
#include "RogueActionEffect.h"
#include "RogueAttributeSet.h"
#include "SharedGameplayTags.h"



URogueActionSystemComponent::URogueActionSystemComponent()
{
	bWantsInitializeComponent = true;
}

void URogueActionSystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// Fallback for Blueprint and CPP having not yet defined a default
	if (Attributes == nullptr)
	{
		Attributes = NewObject<URogueAttributeSet>(this, URogueAttributeSet::StaticClass());
		UE_LOG(LogGame, Warning, TEXT("No default AttributeSet defined. Set using SetDefaultAttributeSet() "
								"during Actor Construction or assign in Blueprint ActionComponent for %s."), *GetNameSafe(GetOwner()));
	}

	for (TFieldIterator<FStructProperty> PropIt(Attributes->GetClass()); PropIt; ++PropIt)
	{
		FRogueAttribute* FoundAttribute = PropIt->ContainerPtrToValuePtr<FRogueAttribute>(Attributes);
		
		FName AttributeTagName = FName("Attribute." + PropIt->GetName());
		FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag(AttributeTagName);

		CachedAttributes.Add(AttributeTag, FoundAttribute);
	}

	for (TSubclassOf<URogueAction> ActionClass : DefaultActions)
	{
		if (ensure(ActionClass))
		{
			GrantAction(ActionClass);
		}
	}
}

void URogueActionSystemComponent::SetDefaultAttributeSet(TSubclassOf<URogueAttributeSet> AttributeSetClass)
{
	check(!HasBeenInitialized());

	// Only available during constructors of UObjects
	FObjectInitializer& ObjectInitializer = FObjectInitializer::Get();
	Attributes = Cast<URogueAttributeSet>(ObjectInitializer.CreateDefaultSubobject(this, TEXT("Attributes"), AttributeSetClass, AttributeSetClass));
}

void URogueActionSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	Attributes->InitializeAttributes();
}

void URogueActionSystemComponent::GrantAction(TSubclassOf<URogueAction> NewActionClass)
{
	const bool bIsEffectClass = NewActionClass->IsChildOf(URogueActionEffect::StaticClass());
	if (bIsEffectClass)
	{
		// Find existing debuff by class, you could have different 'stacking behavior' eg. allowing one debuff class PER instigator
		// Note: Buffs and Actions may desire their own individual arrays when expanding on the Action System
		for (URogueAction* Action : Actions)
		{
			if (URogueActionEffect* Effect = Cast<URogueActionEffect>(Action))
			{
				if (Effect->GetClass() == NewActionClass)
				{
					Effect->IncrementStackSize();
					return;
				}
			}
		}
	}
	
	URogueAction* NewAction = NewObject<URogueAction>(this, NewActionClass);
	Actions.Add(NewAction);

	if (bIsEffectClass)
	{
		// Sanity check that buffs are allowed to run. We do not handle this case yet
		ensureMsgf(NewAction->CanStart(), TEXT("Effect can not start CanStart returns FALSE. Case not handled."));

		NewAction->StartAction();
	}
}

void URogueActionSystemComponent::RemoveAction(URogueAction* ActionToRemove)
{
	int32 RemoveCount = Actions.RemoveSingle(ActionToRemove);
	ensure(RemoveCount == 1);
}

void URogueActionSystemComponent::AppendActiveTags(FGameplayTagContainer NewTags)
{
	ActiveGameplayTags.AppendTags(NewTags);
	
	CheckAgainstBlockedTags(NewTags);

	for (FGameplayTag Tag : NewTags)
	{
		GameplayTagUpdated.Broadcast(Tag, 1);
	}
}

void URogueActionSystemComponent::RemoveActiveTags(FGameplayTagContainer TagsToRemove)
{
	int32 PrevCount = ActiveGameplayTags.Num();
	
	ActiveGameplayTags.RemoveTags(TagsToRemove);
	
	ensure((PrevCount - ActiveGameplayTags.Num()) == TagsToRemove.Num());
	
	for (FGameplayTag Tag : TagsToRemove)
	{
		GameplayTagUpdated.Broadcast(Tag, 0);
	}
}

void URogueActionSystemComponent::CheckAgainstBlockedTags(const FGameplayTagContainer& NewTags)
{
	for (URogueAction* Action : Actions)
	{
		if (Action->IsRunning() && NewTags.HasAny(Action->GetBlockedTags()))
		{
			Action->StopAction();
			
			UE_LOGFMT(LogGame, Log, "Stopped {ActionName} due to any matching blocked tag {BlockedTags} for {Owner}",
				("ActionName", Action->GetActionName().ToString()),
				("BlockedTags", NewTags.ToString()),
				("Owner", GetNameSafe(GetOwner())));
		}
	}
}

void URogueActionSystemComponent::StartAction(FGameplayTag InActionName)
{
	for (URogueAction* Action : Actions)
	{
		if (Action->GetActionName() == InActionName)
		{
			if (Action->CanStart())
			{
				Action->StartAction();
			}
			
			return;
		}
	}

	UE_LOG(LogGame, Warning, TEXT("No Action found with name %s"), *InActionName.ToString());
}

void URogueActionSystemComponent::StopAction(FGameplayTag InActionName)
{
	for (URogueAction* Action : Actions)
	{
		if (Action->GetActionName() == InActionName)
		{
			if (Action->IsRunning())
			{
				Action->StopAction();
			}
			return;
		}
	}

	UE_LOG(LogGame, Warning, TEXT("No Action found with name %s"), *InActionName.ToString());
}

void URogueActionSystemComponent::ApplyAttributeChange(FGameplayTag AttributeTag, float Delta, EAttributeModifyType ModifyType)
{
	FRogueAttribute* FoundAttribute = GetAttribute(AttributeTag);
	check(FoundAttribute);

	float OldValue = FoundAttribute->GetValue();

	switch (ModifyType)
	{
	case Base:
		FoundAttribute->Base += Delta;
		break;
	case Modifier:
		FoundAttribute->Modifier += Delta;
		break;
	case OverrideBase:
		FoundAttribute->Base = Delta;
		break;
	default:
		check(false);
	}

	Attributes->PostAttributeChanged();

	// Native C++ listeners
	if (FOnAttributeChanged* Event = AttributeListeners.Find(AttributeTag))
	{
		Event->Broadcast(AttributeTag, FoundAttribute->GetValue(), OldValue);
	}
	// Blueprint listeners
	if (TArray<FOnAttributeDynamicChanged>* Events = AttributeDynamicListeners.Find(AttributeTag))
	{
		for (int i = Events->Num() - 1; i >= 0; --i)
		{
			FOnAttributeDynamicChanged& Event = (*Events)[i];
			bool bIsBound = Event.ExecuteIfBound(AttributeTag, FoundAttribute->GetValue(), OldValue);
			if (!bIsBound)
			{
				Events->RemoveAt(i);
				UE_LOG(LogGame, Log, TEXT("Cleaned up expired attribute delegate for %s"), *GetNameSafe(GetOwner()));
			}
		}
	}

	UE_LOGFMT(LogGame, Log, "Attribute: {0}, New: {1}, Old: {2}",
		AttributeTag.ToString(),
		FoundAttribute->GetValue(),
		OldValue);
}

FRogueAttribute* URogueActionSystemComponent::GetAttribute(FGameplayTag InAttributeTag) const
{
	FRogueAttribute* const* FoundAttribute = CachedAttributes.Find(InAttributeTag);

	return *FoundAttribute;
}

float URogueActionSystemComponent::GetAttributeValue(FGameplayTag InAttributeTag) const
{
	FRogueAttribute* FoundAttribute = GetAttribute(InAttributeTag);

	return FoundAttribute->GetValue();
}

FOnAttributeChanged& URogueActionSystemComponent::GetAttributeListener(FGameplayTag AttributeTag)
{
	return AttributeListeners.FindOrAdd(AttributeTag);
}

void URogueActionSystemComponent::AddDynamicAttributeListener(FOnAttributeDynamicChanged Event,	FGameplayTag AttributeTag)
{
	TArray<FOnAttributeDynamicChanged>& Events = AttributeDynamicListeners.FindOrAdd(AttributeTag);
	Events.Add(Event);
}

void URogueActionSystemComponent::RemoveDynamicAttributeListener(FOnAttributeDynamicChanged Event)
{
	for (TPair<FGameplayTag, TArray<FOnAttributeDynamicChanged>>& Listener : AttributeDynamicListeners)
	{
		if (Listener.Value.RemoveSingle(Event) > 0)
		{
			UE_LOG(LogGame, Warning, TEXT("successfully removed blueprint binding."));
			break;
		}
	}
}

