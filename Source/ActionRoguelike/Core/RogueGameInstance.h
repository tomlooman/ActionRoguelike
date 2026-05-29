// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RogueGameInstance.generated.h"

class ARoguePlayerCharacter;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	
	/*
	 * Local cache of currently alive players
	 */
	//UPROPERTY(Transient)
	//TArray<TWeakObjectPtr<ARoguePlayerCharacter>> AlivePlayers;
	
	/*
	 * Local cache of currently alive enemy monsters
	 */
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<APawn>> AliveMonsters;
	
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;
};
