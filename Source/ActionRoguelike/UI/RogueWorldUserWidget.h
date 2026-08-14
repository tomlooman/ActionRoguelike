// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RogueWorldUserWidget.generated.h"

class UOverlay;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueWorldUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
		
	UPROPERTY(BlueprintReadOnly, meta = (ExposeOnSpawn))
	TObjectPtr<USceneComponent> OwningComponent;
	
	void AddToRootCanvas();
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ParentOverlay;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
