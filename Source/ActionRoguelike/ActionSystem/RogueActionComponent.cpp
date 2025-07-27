// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionSystem/RogueActionComponent.h"
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
	FillAttributeCache();
	
	Super::InitializeComponent();
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
	FRogueAttribute** FoundAttribute = AttributeCache.Find(InAttributeTag);
	if (FoundAttribute)
	{
		return *FoundAttribute;
	}

	return nullptr;
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
	FRogueAttribute* Attribute = GetAttribute(Modification.AttributeTag);
	if (Attribute == nullptr)
	{
		UE_LOG(LogGame, Warning, TEXT("Attribute (%s) not found on Actor (%s)."), *Modification.AttributeTag.ToString(), *GetNameSafe(GetOwner()));
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
	FRogueAttributeSet* RogueSet = AttributeSet.GetMutablePtr<FRogueAttributeSet>();
	RogueSet->PostAttributeChanged();

	// With clamping inside the attribute (or a zero delta) no real change might have occured
	if (!FMath::IsNearlyEqual(OriginalValue, Attribute->GetValue()))
	{
		Attribute->OnAttributeChanged.Broadcast(Attribute->GetValue(), Modification);
		return true;
	}
	
	// no actual change occured
	return false;
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


void URogueActionComponent::K2_AddAttributeListener(FGameplayTag AttributeTag, FOnAttributeChangedDynamic Event, bool bCallImmediately /*= false*/)
{
	if (!AttributeTag.IsValid())
	{
		UE_LOG(LogGame, Log, TEXT("No valid GameplayTag specified in AddAttributeListener for %s"), *GetNameSafe(GetOwner()));
		return;
	}
	
	FRogueAttribute* FoundAttribute = GetAttribute(AttributeTag);

	// An "Wrapper" to make the binding easier to use with blueprint (returns handle to unbind from Blueprint instance if needed)
	// Blueprint graph can manually unbind using K2_RemoveAttributeListener, otherwise the binding will clean up when the attribute change
	// triggers and we can no longer find a valid binding to the original blueprint delegate (meaning ExecuteIfBound returns false below)
	FDelegateHandle Handle = FoundAttribute->OnAttributeChanged.AddLambda([Event, FoundAttribute, this](float NewValue, FAttributeModification AttriMod)
	{
		SCOPED_NAMED_EVENT(Blueprint_OnAttributeChanged, FColor::Blue);
		// This lamba is executed anytime we trigger the attribute change.

		// This is the blueprint event - May no longer be bound if the blueprint instance has been destroyed/GC'ed
		bool bIsBound = Event.ExecuteIfBound(NewValue, AttriMod);

		// We instance was deleted, the event is no longer valid
		if (!bIsBound)
		{
			FDelegateHandle Handle = *DynamicDelegateHandles.Find(Event);
			FoundAttribute->OnAttributeChanged.Remove(Handle);
		}
	});

	// Keep track so it can be cleaned up if blueprint owner is deleted
	DynamicDelegateHandles.Add(Event, Handle);

	// Calling immediately is convenient for setting up initial states like in UI
	if (bCallImmediately)
	{
		// @todo: maybe change EAttributeModifyType?
		FAttributeModification AttriMod = FAttributeModification(AttributeTag,
			0.0f, this, GetOwner(), EAttributeModifyType::Invalid, FGameplayTagContainer());
		
		Event.Execute(FoundAttribute->GetValue(), AttriMod);
	}
}

void URogueActionComponent::K2_RemoveAttributeListener(FOnAttributeChangedDynamic Event)
{
	FDelegateHandle Handle = *DynamicDelegateHandles.Find(Event);

	// Iterate all attributes to find matching handle (alternatively, just pass in the correct attributetag)
	for (auto Attribute : AttributeCache)
	{
		bool bFound = Attribute.Value->OnAttributeChanged.Remove(Handle);

		// @todo: maybe count to max 1 or we have an issue with multiple bindings coming from a blueprint...
		if (bFound)
		{
			// Temp to verify this is all working
			UE_LOG(LogTemp, Log, TEXT("Successfully Removed binding from a blueprint"));
		}
	}
}


void URogueActionComponent::SetDefaultAttributeSet(UScriptStruct* InDefaultType)
{
	// Only allow during init
	check(!HasBeenInitialized());

	AttributeSet = FInstancedStruct(InDefaultType);
	check(AttributeSet.GetScriptStruct());

	FillAttributeCache();
}


void URogueActionComponent::FillAttributeCache()
{
	// Depending on when we set the attributecache, it might still be nullptr when called
	// Blueprint might directly set the type on the component, where C++ may call SetAttributeSet
	if (AttributeSet.GetScriptStruct() == nullptr)
	{
		return;
	}
	
	TRACE_CPUPROFILER_EVENT_SCOPE(ActionComponent::CacheAttributes);

	// @todo: preallocate to the correct size if we know the nr of properties in the struct
	AttributeCache.Empty();

	for (TFieldIterator<FStructProperty> PropertyIt(AttributeSet.GetScriptStruct()); PropertyIt; ++PropertyIt)
	{
		const FRogueAttribute* FoundAttribute = PropertyIt->ContainerPtrToValuePtr<FRogueAttribute>(AttributeSet.GetMemory());

		// Build the tag "Attribute.Health" where "Health" is the variable name of the RogueAttribute we just iterated
		FString TagName = TEXT("Attribute." + PropertyIt->GetName());
		FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag(FName(TagName));

		AttributeCache.Add(AttributeTag, const_cast<FRogueAttribute*>(FoundAttribute));
	}
}


URogueActionComponent* URogueActionComponent::GetActionComponent(AActor* FromActor)
{
	return URogueGameplayFunctionLibrary::GetActionComponentFromActor(FromActor);
}


void URogueActionComponent::AddAction(AActor* Instigator, TSubclassOf<URogueAction> ActionClass)
{
	// @todo: instead warn earlier about a poorly configured array
	/*if (!ensure(ActionClass))
	{
		return;
	}*/

	// Skip for clients
	if (!GetOwner()->HasAuthority())
	{
		UE_LOGFMT(LogGame, Warning, "Client attempting to AddAction. [Class: {Class}]", GetNameSafe(ActionClass));
		return;
	}

	URogueAction* NewAction = NewObject<URogueAction>(GetOwner(), ActionClass);
	check(NewAction);

	NewAction->Initialize(this);

	Actions.Add(NewAction);

	// New Replicated Objects list (for networking)
	AddReplicatedSubObject(NewAction);

	if (NewAction->IsAutoStart() && ensure(NewAction->CanStart(Instigator)))
	{
		NewAction->StartAction(Instigator);
	}

	// For this mechanism to work, we cant have multiple actions with the same activation tag
	check(!CachedActions.Contains(NewAction->GetActivationTag()));
	CachedActions.Add(NewAction->GetActivationTag(), NewAction);
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
	// Visible in Unreal Insights with namedevents enabled
	SCOPED_NAMED_EVENT(StartActionName, FColor::Green);
	// Alternative, available when cpu channel is specified
	//TRACE_CPUPROFILER_EVENT_SCOPE(StartActionByName);

	// Not yet implemented for clients which needs to fill the array using repnotify array
	check(CachedActions.Num() > 0);

	URogueAction* Action = *CachedActions.Find(ActionName);
	if (Action)
	{
		if (!Action->CanStart(Instigator))
		{
			FString OwnerName = GetOwner()->GetName();
			FString FailedMsg = FString::Printf(TEXT("%s - Failed to run: %s"), *OwnerName, *ActionName.ToString());

			// Limits display in viewport to one per actor instance
			uint64 Key = GetTypeHash(OwnerName);

			GEngine->AddOnScreenDebugMessage(Key, 2.0f, FColor::Red, FailedMsg);
		}

		// Is Client?
		if (!GetOwner()->HasAuthority())
		{
			// Request on the server
			ServerStartAction(Instigator, ActionName);
			// let it continue to start locally too, reduces latency but needs special consideration on this "prediction"
		}

		// Bookmark for Unreal Insights
		//TRACE_BOOKMARK(TEXT("StartAction::%s"), *GetNameSafe(Action));
		
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

				Action->StopAction(Instigator);
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
