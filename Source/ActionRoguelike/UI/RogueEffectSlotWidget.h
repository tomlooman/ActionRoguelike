// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RogueEffectSlotWidget.generated.h"

class URogueActionEffect;
class UImage;
class URogueAction;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueEffectSlotWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> EffectImage;
	
	UPROPERTY(BlueprintReadOnly, Category="Rogue|Effects", meta = (ExposeOnSpawn=true))
	TObjectPtr<URogueActionEffect> EffectInst;

	virtual void NativeConstruct() override;

	void OnLoadIconCompleted(const FTopLevelAssetPath& AssetPath, UObject* LoadedObject, EAsyncLoadingResult::Type Result);

	UFUNCTION()
	void OnEffectEnded(URogueActionComponent* OwningComp, URogueAction* Action);
};
