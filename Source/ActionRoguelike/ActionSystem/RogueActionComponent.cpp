// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionSystem/RogueActionComponent.h"
#include "ActionSystem/RogueAction.h"
#include "Core/RogueGameplayInterface.h"
#include "../ActionRoguelike.h"
#include "Net/UnrealNetwork.h"
#include "RogueTypes.h"
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

	for (TFieldIterator<FStructProperty> PropertyIt(AttributeSet.GetScriptStruct()); PropertyIt; ++PropertyIt)
	{
		const FRogueAttribute* FoundAttribute = PropertyIt->ContainerPtrToValuePtr<FRogueAttribute>(AttributeSet.GetMemory());

		// Build the tag "Attribute.Health" where "Health" is the variable name of the RogueAttribute we just iterated
		FString TagName = TEXT("Attribute." + PropertyIt->GetName());
		FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag(FName(TagName));

		AttributeCache.Add(AttributeTag, FoundAttribute);
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
	// Stop all
	TArray<URogueAction*> ActionsCopy = Actions;
	for (URogueAction* Action : ActionsCopy)
	{
		if (Action->IsRunning())
		{
			Action->StopAction(GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}


bool URogueActionComponent::GetAttribute(FGameplayTag InAttributeTag, FRogueAttribute& OutAttribute)
{
	const FRogueAttribute* FoundAttribute = *AttributeCache.Find(InAttributeTag);
	if (FoundAttribute)
	{
		OutAttribute = *FoundAttribute;
		return true;
	}
	

	// Split the tag to only the attribute name, eg. "Health"
	//FName PropertyName;
	//if (GetAttributeName(InAttributeTag, PropertyName))
	{
		// With Unreal Property/Reflection system we can find and get all data so long as members are marked with UPROPERTY()
		/*FStructProperty* AttributeProp = CastField<FStructProperty>(AttributeSet.GetScriptStruct()->FindPropertyByName(PropertyName));
		if (AttributeProp)
		{
			// Convert the found container data to our attribute struct
			const FRogueAttribute* FoundAttribute = AttributeProp->ContainerPtrToValuePtr<FRogueAttribute>(AttributeSet.GetMemory());
			OutAttribute = *FoundAttribute;
			return true;
		}*/
	}

	return false;
}


bool URogueActionComponent::K2_GetAttribute(FGameplayTag InAttributeTag, float& CurrentValue, float& Base, float& Delta)
{
	FRogueAttribute FoundAttribute;
	if (GetAttribute(InAttributeTag, FoundAttribute))
	{
		CurrentValue = FoundAttribute.GetValue();
		Base = FoundAttribute.Base;
		Delta = FoundAttribute.Delta;
	}

	return false;
}


bool URogueActionComponent::ApplyAttributeChange(FGameplayTag InAttributeTag, FAttributeModification Modification)
{
	FRogueAttribute Attribute;
	GetAttribute(InAttributeTag, Attribute);

	switch (Modification.ModifyType)
	{
		case EAttributeModifyType::AddBase:
			{
				Attribute.Base += Modification.Magnitude;
				break;
			}
		case EAttributeModifyType::AddDelta:
			{
				Attribute.Delta += Modification.Magnitude;
				break;
			}
		case EAttributeModifyType::OverrideBase:
			{
				Attribute.Base = Modification.Magnitude;
				break;
			}
		default:
			// Always fail here so we can address it
			check(false);
	}

	BroadcastAttributeListener(InAttributeTag, Attribute.GetValue(), Modification);
	
	return true;
}


void URogueActionComponent::K2_AddAttributeListener(FGameplayTag AttributeTag, const FOnAttributeChangedDynamic& Event)
{
	//FAttributeDelegateHandle Wrapper;
	//Wrapper.DynamicDelegate = Event;
	//AttributeListeners.Add(TPair<FGameplayTag, FAttributeDelegateHandle>(AttributeTag, Wrapper));


	if (TArray<FAttributeDelegateHandle>* Handles = Listeners.Find(AttributeTag))
	{
		Handles->Add(FAttributeDelegateHandle(Event));
	}
}


FDelegateHandle URogueActionComponent::AddAttributeListener(FGameplayTag AttributeTag, const FOnAttributeChangedNonDynamic& Func)
{
	//FAttributeDelegateHandle Wrapper;
	//Wrapper.Delegate = Func;
	//AttributeListeners.Add(TPair<FGameplayTag, FAttributeDelegateHandle>(AttributeTag, Wrapper));

	if (FOnAttributeChangedList* DelegateList = AttributeListeners.Find(AttributeTag))
	{
		// Append delegate to exist list for specific tag
		DelegateList->Delegates.Add(Func);
	}
	else
	{
		// Did not find any for this tag, create a fresh list
		FOnAttributeChangedList NewList;
		NewList.Delegates.Add(Func);
		AttributeListeners.Add(AttributeTag, NewList);
	}

	return Func.GetHandle();
}


void URogueActionComponent::RemoveAttributeListener(FGameplayTag AttributeTag, FDelegateHandle Handle)
{
	if (FOnAttributeChangedList* DelegateList = AttributeListeners.Find(AttributeTag))
	{
		for (int32 i = 0; i < DelegateList->Delegates.Num(); i++)
		{
			if (Handle == DelegateList->Delegates[i].GetHandle())
			{
				// Clear
				DelegateList->Delegates[i] = nullptr;
				break;
			}
		}
	}
}


void URogueActionComponent::RemoveAttributeListener(FGameplayTag AttributeTag, FAttributeDelegateHandle Handle)
{
	TArray<FAttributeDelegateHandle> DelegateList = *Listeners.Find(AttributeTag);
	check(DelegateList.Num() > 0);
	
	for (int32 i = 0; i < DelegateList.Num(); i++)
	{
		if (Handle == DelegateList[i])
		{
			// Clear
			DelegateList.RemoveAt(i);
			break;
		}
	}
}


void URogueActionComponent::BroadcastAttributeListener(FGameplayTag AttributeTag, float NewValue, const FAttributeModification& AppliedMod)
{
	if (FOnAttributeChangedList* DelegateList = AttributeListeners.Find(AttributeTag))
	{
		for (FOnAttributeChangedNonDynamic& Delegate : DelegateList->Delegates)
		{
			Delegate.Execute(NewValue, AppliedMod);
		}
	}
}


bool URogueActionComponent::GetAttributeName(const FGameplayTag InTag, FName& OutAttributeName)
{
	// Attribute names should reflect the Tag name in project. eg. Grab "Health" (property name) from "Attribute.Health" GameplayTag
	FString LeftStr;
	FString RightStr;
	InTag.ToString().Split(".", &LeftStr, &RightStr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
#if !UE_BUILD_SHIPPING
	if (RightStr.IsEmpty())
	{
		UE_LOG(LogGame, Warning, TEXT("Failed to split GameplayTag (%s) in GetAttribute."), *InTag.ToString());
		return false;
	}
#endif

	OutAttributeName = FName(*RightStr);
	return true;
}


void URogueActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//FString DebugMsg = GetNameSafe(GetOwner()) + " : " + ActiveGameplayTags.ToStringSimple();
	//GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, DebugMsg);

	// Draw All Actions
// 	for (URogueAction* Action : Actions)
// 	{
// 		FColor TextColor = Action->IsRunning() ? FColor::Blue : FColor::White;
// 		FString ActionMsg = FString::Printf(TEXT("[%s] Action: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(Action));
// 
// 		LogOnScreen(this, ActionMsg, TextColor, 0.0f);
// 	}
}


URogueActionComponent* URogueActionComponent::GetComponent(AActor* InActor)
{
	if (InActor && InActor->Implements<URogueGameplayInterface>())
	{
		URogueActionComponent* ActionComp = nullptr;
		if (IRogueGameplayInterface::Execute_GetActionComponent(InActor, ActionComp))
		{
			return ActionComp;
		}
	}

	// @todo: log warn about interface not implemented yet

	// Iterate over all components anyway if not implemented. But warn about this

	return InActor->GetComponentByClass<URogueActionComponent>();
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
}