// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionSystemComponent.h"

#include "RogueAction.h"
#include "RogueAttributeSet.h"


URogueActionSystemComponent::URogueActionSystemComponent()
{
	bWantsInitializeComponent = true;

	AttributeSetClass = URogueAttributeSet::StaticClass();
}

void URogueActionSystemComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Attributes = NewObject<URogueAttributeSet>(this, AttributeSetClass);

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

void URogueActionSystemComponent::GrantAction(TSubclassOf<URogueAction> NewActionClass)
{
	URogueAction* NewAction = NewObject<URogueAction>(this, NewActionClass);
    Actions.Add(NewAction);
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

	UE_LOG(LogTemp, Warning, TEXT("No Action found with name %s"), *InActionName.ToString());
}

void URogueActionSystemComponent::StopAction(FGameplayTag InActionName)
{
	for (URogueAction* Action : Actions)
	{
		if (Action->GetActionName() == InActionName)
		{
			Action->StopAction();
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("No Action found with name %s"), *InActionName.ToString());
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

	UE_LOGFMT(LogTemp, Log, "Attribute: {0}, New: {1}, Old: {2}",
		AttributeTag.ToString(),
		FoundAttribute->GetValue(),
		OldValue);
}

FRogueAttribute* URogueActionSystemComponent::GetAttribute(FGameplayTag InAttributeTag)
{
	FRogueAttribute** FoundAttribute = CachedAttributes.Find(InAttributeTag);

	return *FoundAttribute;
}

