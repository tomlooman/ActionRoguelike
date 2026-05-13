// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionSystem/RogueActionComponent.h"

#include "RogueActionEffect.h"
#include "ActionSystem/RogueAction.h"
#include "../ActionRoguelike.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueActionComponent)


URogueActionComponent::URogueActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;

	SetIsReplicatedByDefault(true);

	// See GDefaultUseSubObjectReplicationList for CVAR to enable by default project-wide
	bReplicateUsingRegisteredSubObjectList = true;
}


void URogueActionComponent::InitializeComponent()
{
	// Call before Super:: as we verify we haven't initialized yet
	if (AttributeSet == nullptr)
	{
		AttributeSet = NewObject<URogueAttributeSet>(this, URogueAttributeSet::StaticClass());
		UE_LOG(LogGame, Warning, TEXT("No default AttributeSet was specified. Set using SetDefaultAttributeSet during Actor construction"
								"or assign in the Blueprint details panel in the ActionComponent of %s"), *GetNameSafe(GetOwner()));
	}

	Super::InitializeComponent();
	
	InitAttributeSet();

	if (GetOwner()->HasAuthority())
	{
		AddReplicatedSubObject(AttributeSet);
	}
}

void URogueActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Server Only
	if (GetOwner()->HasAuthority())
	{
		for (TSubclassOf<URogueAction> ActionClass : DefaultActions)
		{
			AddAction(GetOwner(), ActionClass);
		}
	}
}


void URogueActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAllActions();

	Super::EndPlay(EndPlayReason);
}


FRogueAttribute* URogueActionComponent::GetAttribute(FGameplayTag InAttributeTag)
{
	check(AttributeSet);
		
	FRogueAttribute** FoundAttribute = AttributeSet->AttributeCache.Find(InAttributeTag);
	if (FoundAttribute)
	{
		return *FoundAttribute;
	}

	return nullptr;
}

float URogueActionComponent::GetAttributeValue(FGameplayTag InAttributeTag)
{
	check(AttributeSet);
		
	FRogueAttribute** FoundAttribute = AttributeSet->AttributeCache.Find(InAttributeTag);
	if (FoundAttribute)
	{
		return (*FoundAttribute)->GetValue();
	}

	return 0.0f;
}


bool URogueActionComponent::K2_GetAttribute(FGameplayTag InAttributeTag, float& CurrentValue, float& Base, float& Delta)
{
	if (FRogueAttribute* FoundAttribute = GetAttribute(InAttributeTag))
	{
		CurrentValue = FoundAttribute->GetValue();
		Base = FoundAttribute->Base;
		Delta = FoundAttribute->Modifier;
	}

	return false;
}


bool URogueActionComponent::ApplyAttributeChange(const FAttributeModification& Modification)
{
	if (!GetOwner()->HasAuthority())
	{
		// Skip on clients.
		return false;
	}

	const FGameplayTag AttributeTag = Modification.AttributeTag;
	
	FRogueAttribute* Attribute = GetAttribute(AttributeTag);
	if (Attribute == nullptr)
	{
		UE_LOG(LogGame, Warning, TEXT("Attribute (%s) not found on Actor (%s)."), *AttributeTag.ToString(), *GetNameSafe(GetOwner()));
		return false;
	}

	float OriginalValue = Attribute->GetValue();

	switch (Modification.ModifyType)
	{
		case EAttributeModifyType::AddBase:
			{
				Attribute->Base += Modification.Magnitude;
				break;
			}
		case EAttributeModifyType::AddModifier:
			{
				Attribute->Modifier += Modification.Magnitude;
				break;
			}
		case EAttributeModifyType::OverrideBase:
			{
				Attribute->Base = Modification.Magnitude;
				break;
			}
		default:
			// Always fail here so we can address it
			check(false);
	}
	
	// Allow further modification here for the attribute set
	AttributeSet->PostAttributeChanged();

	// With clamping inside the attribute (or a zero delta) no real change might have occured
	if (!FMath::IsNearlyEqual(OriginalValue, Attribute->GetValue()))
	{
		BroadcastAttributeChanged(AttributeTag, Attribute->GetValue(), Modification);
		
		return true;
	}
	
	// no actual change occured
	return false;
}


void URogueActionComponent::RemoveDynamicAttributeListener(FAttributeChangedDynamicSignature Event)
{
	// Iterates all tags and arrays, could be faster if we specify the Tag in the function to know where to remove it.
	for (TPair<FGameplayTag, TArray<FAttributeChangedDynamicSignature>>& Listener : AttributeBlueprintListeners)
	{
		if (Listener.Value.RemoveSingle(Event) > 0)
		{
			// found it, skip checking the rest
			
			// Temp to verify this is all working
			UE_LOG(LogTemp, Log, TEXT("Successfully Removed binding from a blueprint"));
			break;
		}
	}
}


bool URogueActionComponent::ApplyAttributeChange(FGameplayTag InAttributeTag, float InMagnitude, AActor* Instigator, EAttributeModifyType ModType, FGameplayTagContainer InContextTags)
{
	FAttributeModification AttriMod = FAttributeModification(
		InAttributeTag,
		InMagnitude,
		this,
		Instigator,
		ModType,
		InContextTags);

	return ApplyAttributeChange(AttriMod);
}


void URogueActionComponent::OnRep_AttributeSet()
{
	InitAttributeSet();
}


void URogueActionComponent::InitAttributeSet()
{
	AttributeSet->InitializeAttributes(this);
}


FAttributeChangedSignature& URogueActionComponent::GetAttributeListenerDelegate(FGameplayTag InTag)
{
	return AttributeListenerMap.FindOrAdd(InTag);
}

void URogueActionComponent::AddDynamicAttributeListener(FAttributeChangedDynamicSignature Event, FGameplayTag InTag, bool bCallImmediately /*= false*/)
{
	TArray<FAttributeChangedDynamicSignature>& Events = AttributeBlueprintListeners.FindOrAdd(InTag);
	Events.Add(Event);

	if (bCallImmediately)
	{
		// Fill with minimal info available, useful for setting up initial states in UI etc.
		const FAttributeModification AttriMod = FAttributeModification(InTag,
			0.0f,
			this,
			GetOwner(),
			EAttributeModifyType::Invalid,
			FGameplayTagContainer());

		Event.Execute(GetAttributeValue(InTag), AttriMod);
	}
}

void URogueActionComponent::BroadcastAttributeChanged(FGameplayTag InTag, float InNewValue, FAttributeModification InModification)
{
	// Broadcast for all native listeners
	if (FAttributeChangedSignature* Event = AttributeListenerMap.Find(InTag))
	{
		Event->Broadcast(InNewValue, InModification);
	}
		
	// Broadcast any Blueprint listeners
	if (TArray<FAttributeChangedDynamicSignature>* Events = AttributeBlueprintListeners.Find(InTag))
	{
		// Reverse-for to allow cleanup for any events no longer bound to blueprint instances
		for (int i = Events->Num() - 1; i >= 0; --i)
		{
			FAttributeChangedDynamicSignature& Event = (*Events)[i];
			bool bIsBound = Event.ExecuteIfBound(InNewValue, InModification);
			if (!bIsBound)
			{
				Events->RemoveAt(i);
				UE_LOG(LogGame, Log, TEXT("Cleaned up expired attribute delegate for %s"), *GetNameSafe(GetOwner()));
			}
		}
	}
}


void URogueActionComponent::SetDefaultAttributeSet(const TSubclassOf<URogueAttributeSet>& InNewClass)
{
	// Only allow during init
	check(!HasBeenInitialized());

	const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get();
	AttributeSet = Cast<URogueAttributeSet>(ObjectInitializer.CreateDefaultSubobject(this, TEXT("Attributes"), InNewClass, InNewClass));
}


URogueActionComponent* URogueActionComponent::GetActionComponent(AActor* FromActor)
{
	return URogueGameplayFunctionLibrary::GetActionComponentFromActor(FromActor);
}


void URogueActionComponent::AddAction(AActor* Instigator, TSubclassOf<URogueAction> ActionClass)
{
	// Skip for clients
	if (!GetOwner()->HasAuthority())
	{
		UE_LOGFMT(LogGame, Warning, "Client attempting to AddAction. [Class: {Class}]", GetNameSafe(ActionClass));
		return;
	}
	
	const bool bIsEffectClass = ActionClass->IsChildOf(URogueActionEffect::StaticClass());
	
	// First check if we can increment a buff stack instead of adding a fresh Action(Effect)
	if (bIsEffectClass)
	{
		// @todo: name is not quite accurate for effect descriptions (something like Status.Burning)
		FGameplayTag Tag = ActionClass->GetDefaultObject<URogueAction>()->GetActivationTag();
		// Buffs may not be setting these TAGs yet.
		ensure(Tag.IsValid());

		// Effect may not be present
		if (TObjectPtr<URogueAction>* FoundItem = CachedActions.Find(Tag))
		{
			if (URogueActionEffect* FoundEffect = Cast<URogueActionEffect>(*FoundItem))
			{
				FoundEffect->IncrementStackSize();
				return;			
			}
		}
	}

	URogueAction* NewAction = NewObject<URogueAction>(GetOwner(), ActionClass);
	check(NewAction);

	NewAction->Initialize(this);

	Actions.Add(NewAction);

	// New Replicated Objects list (for networking)
	AddReplicatedSubObject(NewAction);

	// Auto start all buffs, if allowed
	if (bIsEffectClass && ensure(NewAction->CanStart(Instigator)))
	{
		NewAction->StartAction(Instigator);
	}

	// For this mechanism to work, we cant have multiple actions with the same activation tag
	// Only for actions with activation tag, buffs will not have those set
	if (NewAction->GetActivationTag().IsValid())
	{
		check(!CachedActions.Contains(NewAction->GetActivationTag()));
		CachedActions.Add(NewAction->GetActivationTag(), NewAction);
	}
}


void URogueActionComponent::RemoveAction(URogueAction* ActionToRemove)
{
	if (!ensure(ActionToRemove && !ActionToRemove->IsRunning()))
	{
		return;
	}

	RemoveReplicatedSubObject(ActionToRemove);

	Actions.Remove(ActionToRemove);

	CachedActions.Remove(ActionToRemove->GetActivationTag());
}


URogueAction* URogueActionComponent::GetAction(TSubclassOf<URogueAction> ActionClass) const
{
	for (URogueAction* Action : Actions)
	{
		if (Action->IsA(ActionClass))
		{
			return Action;
		}
	}

	return nullptr;
}


bool URogueActionComponent::StartActionByName(AActor* Instigator, FGameplayTag ActionName)
{
	//SCOPED_NAMED_EVENT(StartActionName, FColor::Green); // Visible in Unreal Insights with namedevents enabled
	TRACE_CPUPROFILER_EVENT_SCOPE(StartActionByName); // Alternative, available when Insights "CPU" channel is specified
	
	check(CachedActions.Num() > 0);
	if (TObjectPtr<URogueAction>* ActionPtr = CachedActions.Find(ActionName))
	{
		URogueAction* Action = ActionPtr->Get();
		
		if (!Action->CanStart(Instigator))
		{
			FString OwnerName = GetOwner()->GetName();
			FString FailedMsg = FString::Printf(TEXT("%s - Failed to run: %s"), *OwnerName, *ActionName.ToString());

			// Limits display in viewport to one per actor instance
			uint64 Key = GetTypeHash(OwnerName);

			GEngine->AddOnScreenDebugMessage(Key, 2.0f, FColor::Red, FailedMsg);
			return false;
		}

		// Bookmark for Unreal Insights
		//TRACE_BOOKMARK(TEXT("StartAction::%s"), *GetNameSafe(Action));

		// Is Client?
		if (!GetOwner()->HasAuthority())
		{
			// Request on the server
			ServerStartAction(Instigator, ActionName);
		}
		else
		{
			// Scoped within the curly braces. the _FSTRING variant adds additional tracing overhead due to grabbing the class name every time
			SCOPED_NAMED_EVENT_FSTRING(Action->GetClass()->GetName(), FColor::White);

			Action->StartAction(Instigator);
		}
		
		return true;
	}

	return false;
}


bool URogueActionComponent::StopActionByName(AActor* Instigator, FGameplayTag ActionName)
{
	for (URogueAction* Action : Actions)
	{
		if (Action->GetActivationTag() == ActionName)
		{
			if (Action->IsRunning())
			{
				// Is Client?
				if (!GetOwner()->HasAuthority())
				{
					ServerStopAction(Instigator, ActionName);
				}
				else
				{
					Action->StopAction(Instigator);
				}

				return true;
			}
		}
	}

	return false;
}

void URogueActionComponent::StopAllActions()
{
	// Stop all
	TArray<URogueAction*> ActionsCopy = Actions;
	for (URogueAction* Action : ActionsCopy)
	{
		if (Action->IsRunning())
		{
			Action->StopAction(GetOwner());
		}
	}
}


void URogueActionComponent::ServerStartAction_Implementation(AActor* Instigator, FGameplayTag ActionName)
{
	StartActionByName(Instigator, ActionName);
}


void URogueActionComponent::ServerStopAction_Implementation(AActor* Instigator, FGameplayTag ActionName)
{
	StopActionByName(Instigator, ActionName);
}


void URogueActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URogueActionComponent, Actions);
	DOREPLIFETIME(URogueActionComponent, AttributeSet);
}


void URogueActionComponent::OnRep_Actions()
{
	// Expect this only for clients, host does it during add/remove action
	check(IsNetMode(NM_Client));
	
	// Reset
	CachedActions.Empty(Actions.Num());

	for (URogueAction* Action : Actions)
	{
		CachedActions.Add(Action->GetActivationTag(), Action);
	}
}
