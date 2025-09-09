// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueActionSystemComponent.h"


URogueActionSystemComponent::URogueActionSystemComponent()
{

}

void URogueActionSystemComponent::ApplyHealthChange(float InValueChange)
{
	Attributes.Health += InValueChange;

	UE_LOG(LogTemp, Log, TEXT("New Health: %f"), Attributes.Health);
}
