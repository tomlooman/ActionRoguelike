// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAICharacter.h"

#include "ActionSystem/RogueActionSystemComponent.h"


ARogueAICharacter::ARogueAICharacter()
{

	ActionSystemComponent = CreateDefaultSubobject<URogueActionSystemComponent>(TEXT("ActionSystemComp"));

}

float ARogueAICharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	ActionSystemComponent->ApplyHealthChange(-ActualDamage);

	return ActualDamage;
}
