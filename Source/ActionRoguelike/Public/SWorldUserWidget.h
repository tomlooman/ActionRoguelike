// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SWorldUserWidget.generated.h"


class UOverlay;

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API USWorldUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> ParentOverlay;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	virtual void NativeConstruct() override;

	bool bWasOnScreen = true;

public:

	/* Find the root canvas panel (expected in a 'main hud' style widget) and add new widget as child. */
	UFUNCTION(BlueprintCallable, Category= UI)
	static void AddToRootCanvasPanel(UUserWidget* InNewWidget);

	UPROPERTY(EditAnywhere, Category = "UI")
	FVector WorldOffset;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "UI", meta = (ExposeOnSpawn=true))
	TObjectPtr<AActor> AttachedActor;
};
