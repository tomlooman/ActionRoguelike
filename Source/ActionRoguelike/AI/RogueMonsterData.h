// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RogueMonsterData.generated.h"

class URogueAction;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueMonsterData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<APawn> MonsterClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<URogueAction>> Actions;
	
	// Example: Behavior Tree from AI Controller
};
