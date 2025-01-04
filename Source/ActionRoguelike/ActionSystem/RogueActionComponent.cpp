// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionSystem/RogueActionComponent.h"
#include "ActionSystem/RogueAction.h"
#include "Core/RogueGameplayInterface.h"
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
	Super::InitializeComponent();

	{
		TRACE_CPUPROFILER_EVENT_SCOPE(CacheAllAttributes);

		for (TFieldIterator<FStructProperty> PropertyIt(AttributeSet.GetScriptStruct()); PropertyIt; ++PropertyIt)
		{
			const FRogueAttribute* FoundAttribute = PropertyIt->ContainerPtrToValuePtr<FRogueAttribute>(AttributeSet.GetMemory());

			// Build the tag "Attribute.Health" where "Health" is the variable name of the RogueAttribute we just iterated
			FString TagName = TEXT("Attribute." + PropertyIt->GetName());
			FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag(FName(TagName));

			AttributeCache.Add(AttributeTag, const_cast<FRogueAttribute*>(FoundAttribute));
		}
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

	// With clamping inside the attribute (or a zero delta) no real change might have occured
	if (!FMath::IsNearlyEqual(OriginalValue, Attribute->GetValue()))
	{
		Attribute->OnAttributeChanged.Broadcast(Attribute->GetValue(), Modification);
		return true;
	}
	
	// no actual change occured
	return false;
}


void URogueActionComponent::K2_AddAttributeListener(FGameplayTag AttributeTag, FOnAttributeChangedDynamic Event, bool bCallImmediately /*= false*/)
{
	if (!AttributeTag.IsValid())
	{
		UE_LOG(LogGame, Log, TEXT("No valid GameplayTag specified in AddAttributeListener for %s"), *GetNameSafe(GetOwner()));
		return;
	}
	
	FRogueAttribute* FoundAttribute = GetAttribute(AttributeTag);

	// An unusual "Wrapper" to make the binding easier in blueprint (returns handle to unbind from Blueprint if needed)
	FDelegateHandle Handle = FoundAttribute->OnAttributeChanged.AddLambda([Event, FoundAttribute, this](float NewValue, FAttributeModification AttriMod)
	{
		bool bIsBound = Event.ExecuteIfBound(NewValue, AttriMod);

		// We instance was deleted, the event is no longer valid
		if (!bIsBound)
		{
			FDelegateHandle Handle = *DynamicDelegateHandles.Find(Event);
			FoundAttribute->OnAttributeChanged.Remove(Handle);
		}
	});

	// Keep track so it can be cleaned up if blueprint owning is deleted
	DynamicDelegateHandles.Add(Event, Handle);

	// Calling immediately is convenient for setting up initial states like in UI
	if (bCallImmediately)
	{
		// @todo: maybe change EAttributeModifyType?
		FAttributeModification AttriMod = FAttributeModification(AttributeTag,
			0.0f, this, GetOwner(), EAttributeModifyType::Invalid);
		
		Event.Execute(FoundAttribute->GetValue(), AttriMod);
	}
}


void URogueActionComponent::SetDefaultAttributeSet(UScriptStruct* InDefaultType)
{
	// @todo: maybe add safeguards to only allow this during init. We don't want to swap out set during gameplay
	AttributeSet = FInstancedStruct(InDefaultType);
}


URogueActionComponent* URogueActionComponent::GetActionComponent(AActor* FromActor)
{
	return URogueGameplayFunctionLibrary::GetActionComponentFromActor(FromActor);
}


void URogueActionComponent::AddAction(AActor* Instigator, TSubclassOf<URogueAction> ActionClass)
{
	if (!ensure(ActionClass))
	{
		return;
	}

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
}


void URogueActionComponent::RemoveAction(URogueAction* ActionToRemove)
{
	if (!ensure(ActionToRemove && !ActionToRemove->IsRunning()))
	{
		return;
	}

	RemoveReplicatedSubObject(ActionToRemove);

	Actions.Remove(ActionToRemove);
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


	for (URogueAction* Action : Actions)
	{
		if (Action->GetActivationTag() == ActionName)
		{
			if (!Action->CanStart(Instigator))
			{
				FString FailedMsg = FString::Printf(TEXT("Failed to run: %s"), *ActionName.ToString());
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailedMsg);
				continue;
			}

			// Is Client?
			if (!GetOwner()->HasAuthority())
			{
				ServerStartAction(Instigator, ActionName);
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