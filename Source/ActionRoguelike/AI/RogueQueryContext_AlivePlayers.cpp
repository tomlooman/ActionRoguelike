// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueQueryContext_AlivePlayers.h"

#include "EngineUtils.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "Player/RoguePlayerCharacter.h"



void URogueQueryContext_AlivePlayers::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	Super::ProvideContext(QueryInstance, ContextData);
	
	TArray<AActor*> AlivePlayers;
		
	for (ARoguePlayerCharacter* Player : TActorRange<ARoguePlayerCharacter>(QueryInstance.World))
	{
		if (Player && URogueGameplayFunctionLibrary::IsAlive(Player))
		{
			AlivePlayers.Add(Player);
		}
	}
		
	UEnvQueryItemType_Actor::SetContextHelper(ContextData, AlivePlayers);
}
