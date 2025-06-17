// Fill out your copyright notice in the Description page of Project Settings.


#include "Development/RogueCheatManager.h"

#include "EngineUtils.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "SaveSystem/RogueSaveGameSettings.h"
#include "AI/RogueAICharacter.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"


void URogueCheatManager::HealSelf(float Amount /* = 100 */)
{
	APlayerController* MyPC = GetOuterAPlayerController();

	if (APawn* MyPawn = MyPC->GetPawn())
	{
		URogueActionComponent* ActionComp = URogueActionComponent::GetActionComponent(MyPawn);
		ActionComp->ApplyAttributeChange(SharedGameplayTags::Attribute_Health, Amount, MyPawn, EAttributeModifyType::AddBase);
	}
}


void URogueCheatManager::KillAll()
{
	for (ARogueAICharacter* Bot : TActorRange<ARogueAICharacter>(GetWorld()))
	{
		URogueGameplayFunctionLibrary::KillActor(Bot);
	}
}


void URogueCheatManager::DeleteSaveGame()
{
	const URogueSaveGameSettings* SGSettings = GetDefault<URogueSaveGameSettings>();
	UGameplayStatics::DeleteGameInSlot(SGSettings->SaveSlotName, 0);
}
