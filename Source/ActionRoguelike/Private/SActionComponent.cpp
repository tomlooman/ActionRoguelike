// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionComponent.h"
#include "SAction.h"
#include "SGameplayInterface.h"
#include "../ActionRoguelike.h"
#include "Net/UnrealNetwork.h"
#include "RogueTypes.h"
#include "Engine/ActorChannel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SActionComponent)


USActionComponent::USActionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	// See GDefaultUseSubObjectReplicationList for CVAR to enable by default project-wide
	bReplicateUsingRegisteredSubObjectList = true;
}

bool USActionComponent::GetAttribute(FGameplayTag InAttributeTag, FRogueAttribute& OutAttribute)
{
	// Split the tag to only the attribute name, eg. "Health"
	FName PropertyName;
	if (GetAttributeName(InAttributeTag, PropertyName))
	{
		// With Unreal Property/Reflection system we can find and get all data so long as members are marked with UPROPERTY()
		FStructProperty* AttributeProp = CastField<FStructProperty>(AttributeSet.GetScriptStruct()->FindPropertyByName(PropertyName));
		if (AttributeProp)
		{
			// Convert the found container data to our attribute struct
			const FRogueAttribute* FoundAttribute = AttributeProp->ContainerPtrToValuePtr<FRogueAttribute>(AttributeSet.GetMemory());
			OutAttribute = *FoundAttribute;
			return true;
		}
	}

	return false;
}


bool USActionComponent::K2_GetAttribute(FGameplayTag InAttributeTag, float& CurrentValue, float& Base, float& Delta)
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


bool USActionComponent::ApplyAttributeChange(FGameplayTag InAttributeTag, FAttributeModification Modification)
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


void USActionComponent::K2_AddAttributeListener(FGameplayTag AttributeTag, const FOnAttributeChangedDynamic& Event)
{
	//FAttributeDelegateHandle Wrapper;
	//Wrapper.DynamicDelegate = Event;
	//AttributeListeners.Add(TPair<FGameplayTag, FAttributeDelegateHandle>(AttributeTag, Wrapper));


	if (TArray<FAttributeDelegateHandle>* Handles = Listeners.Find(AttributeTag))
	{
		Handles->Add(FAttributeDelegateHandle(Event));
	}
}


FDelegateHandle USActionComponent::AddAttributeListener(FGameplayTag AttributeTag, const FOnAttributeChangedNonDynamic& Func)
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


void USActionComponent::RemoveAttributeListener(FGameplayTag AttributeTag, FDelegateHandle Handle)
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


void USActionComponent::RemoveAttributeListener(FGameplayTag AttributeTag, FAttributeDelegateHandle Handle)
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


void USActionComponent::BroadcastAttributeListener(FGameplayTag AttributeTag, float NewValue, const FAttributeModification& AppliedMod)
{
	if (FOnAttributeChangedList* DelegateList = AttributeListeners.Find(AttributeTag))
	{
		for (FOnAttributeChangedNonDynamic& Delegate : DelegateList->Delegates)
		{
			Delegate.Execute(NewValue, AppliedMod);
		}
	}
}


bool USActionComponent::GetAttributeName(const FGameplayTag InTag, FName& OutAttributeName)
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


void USActionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Server Only
	if (GetOwner()->HasAuthority())
	{
		for (TSubclassOf<USAction> ActionClass : DefaultActions)
		{
			AddAction(GetOwner(), ActionClass);
		}
	}
}


void USActionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Stop all
	TArray<USAction*> ActionsCopy = Actions;
	for (USAction* Action : ActionsCopy)
	{
		if (Action->IsRunning())
		{
			Action->StopAction(GetOwner());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void USActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//FString DebugMsg = GetNameSafe(GetOwner()) + " : " + ActiveGameplayTags.ToStringSimple();
	//GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, DebugMsg);

	// Draw All Actions
// 	for (USAction* Action : Actions)
// 	{
// 		FColor TextColor = Action->IsRunning() ? FColor::Blue : FColor::White;
// 		FString ActionMsg = FString::Printf(TEXT("[%s] Action: %s"), *GetNameSafe(GetOwner()), *GetNameSafe(Action));
// 
// 		LogOnScreen(this, ActionMsg, TextColor, 0.0f);
// 	}
}


USActionComponent* USActionComponent::GetComponent(AActor* InActor)
{
	if (InActor && InActor->Implements<USGameplayInterface>())
	{
		USActionComponent* ActionComp = nullptr;
		if (ISGameplayInterface::Execute_GetActionComponent(InActor, ActionComp))
		{
			return ActionComp;
		}
	}

	// @todo: log warn about interface not implemented yet

	// Iterate over all components anyway if not implemented. But warn about this

	return InActor->GetComponentByClass<USActionComponent>();
}

void USActionComponent::AddAction(AActor* Instigator, TSubclassOf<USAction> ActionClass)
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

	USAction* NewAction = NewObject<USAction>(GetOwner(), ActionClass);
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


void USActionComponent::RemoveAction(USAction* ActionToRemove)
{
	if (!ensure(ActionToRemove && !ActionToRemove->IsRunning()))
	{
		return;
	}

	RemoveReplicatedSubObject(ActionToRemove);

	Actions.Remove(ActionToRemove);
}


USAction* USActionComponent::GetAction(TSubclassOf<USAction> ActionClass) const
{
	for (USAction* Action : Actions)
	{
		if (Action->IsA(ActionClass))
		{
			return Action;
		}
	}

	return nullptr;
}


bool USActionComponent::StartActionByName(AActor* Instigator, FGameplayTag ActionName)
{
	// Visible in Unreal Insights with namedevents enabled
	SCOPED_NAMED_EVENT(StartActionName, FColor::Green);
	// Alternative, available when cpu channel is specified
	//TRACE_CPUPROFILER_EVENT_SCOPE(StartActionByName);


	for (USAction* Action : Actions)
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
			TRACE_BOOKMARK(TEXT("StartAction::%s"), *GetNameSafe(Action));
			
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


bool USActionComponent::StopActionByName(AActor* Instigator, FGameplayTag ActionName)
{
	for (USAction* Action : Actions)
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


void USActionComponent::ServerStartAction_Implementation(AActor* Instigator, FGameplayTag ActionName)
{
	StartActionByName(Instigator, ActionName);
}


void USActionComponent::ServerStopAction_Implementation(AActor* Instigator, FGameplayTag ActionName)
{
	StopActionByName(Instigator, ActionName);
}


void USActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USActionComponent, Actions);
}