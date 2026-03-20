// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RogueWorldUserWidget.h"
#include "RogueDamageNumberWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueDamageNumberWidget : public URogueWorldUserWidget
{
	GENERATED_BODY()

public:

	void SetDamageAmount(float InNewAmount);
	
protected:

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DamageTextBlock;
};
