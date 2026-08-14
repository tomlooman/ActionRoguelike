// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RogueMonsterData.generated.h"

class URogueWorldUserWidget;
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin=0))
	int32 LootCoins = 100;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<URogueWorldUserWidget> HealthWidgetClass;
	
	// Example: Behavior Tree from AI Controller
};
