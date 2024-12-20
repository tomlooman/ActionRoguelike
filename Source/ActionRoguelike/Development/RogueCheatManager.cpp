// Fill out your copyright notice in the Description page of Project Settings.


#include "Development/RogueCheatManager.h"

#include "EngineUtils.h"
#include "ActionSystem/RogueAttributeComponent.h"
#include "SaveSystem/RogueSaveGameSettings.h"
#include "AI/RogueAICharacter.h"
#include "Kismet/GameplayStatics.h"


void URogueCheatManager::HealSelf(float Amount /* = 100 */)
{
	APlayerController* MyPC = GetOuterAPlayerController();

	if (APawn* MyPawn = MyPC->GetPawn())
	{
		URogueAttributeComponent* AttributeComp = MyPawn->FindComponentByClass<URogueAttributeComponent>();
		AttributeComp->ApplyHealthChange(MyPawn, Amount);
	}
}


void URogueCheatManager::KillAll()
{
	for (ARogueAICharacter* Bot : TActorRange<ARogueAICharacter>(GetWorld()))
	{
		URogueAttributeComponent* AttributeComp = URogueAttributeComponent::GetAttributes(Bot);
		if (ensure(AttributeComp) && AttributeComp->IsAlive())
		{
			AttributeComp->Kill(GetOuterAPlayerController()->GetPawn());
		}
	}
}


void URogueCheatManager::DeleteSaveGame()
{
	const URogueSaveGameSettings* SGSettings = GetDefault<URogueSaveGameSettings>();
	UGameplayStatics::DeleteGameInSlot(SGSettings->SaveSlotName, 0);
}
