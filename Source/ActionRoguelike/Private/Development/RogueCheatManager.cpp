// Fill out your copyright notice in the Description page of Project Settings.


#include "Development/RogueCheatManager.h"

#include "EngineUtils.h"
#include "SAttributeComponent.h"
#include "SSaveGameSettings.h"
#include "AI/SAICharacter.h"
#include "Kismet/GameplayStatics.h"


void URogueCheatManager::HealSelf(float Amount /* = 100 */)
{
	APlayerController* MyPC = GetOuterAPlayerController();

	if (APawn* MyPawn = MyPC->GetPawn())
	{
		USAttributeComponent* AttributeComp = MyPawn->FindComponentByClass<USAttributeComponent>();
		AttributeComp->ApplyHealthChange(MyPawn, Amount);
	}
}


void URogueCheatManager::KillAll()
{
	for (ASAICharacter* Bot : TActorRange<ASAICharacter>(GetWorld()))
	{
		USAttributeComponent* AttributeComp = USAttributeComponent::GetAttributes(Bot);
		if (ensure(AttributeComp) && AttributeComp->IsAlive())
		{
			AttributeComp->Kill(GetOuterAPlayerController()->GetPawn());
		}
	}
}


void URogueCheatManager::DeleteSaveGame()
{
	const USSaveGameSettings* SGSettings = GetDefault<USSaveGameSettings>();
	UGameplayStatics::DeleteGameInSlot(SGSettings->SaveSlotName, 0);
}
