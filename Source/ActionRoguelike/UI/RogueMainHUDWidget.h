// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RogueMainHUDWidget.generated.h"

class UCanvasPanel;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/* Primary Canvas to add all projected widgets such as damage numbers */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> MainCanvasPanel;	
};
