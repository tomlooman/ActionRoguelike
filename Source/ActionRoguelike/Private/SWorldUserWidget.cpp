// Fill out your copyright notice in the Description page of Project Settings.


#include "SWorldUserWidget.h"

#include "ActionRoguelike.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/SizeBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SWorldUserWidget)



void USWorldUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(AttachedActor))
	{
		RemoveFromParent();

		UE_LOGFMT(LogGame, Warning, "AttachedActor no longer valid, removing Health Widget.");
		return;
	}

	FVector2D ScreenPosition;
	bool bIsOnScreen = UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), AttachedActor->GetActorLocation() + WorldOffset, ScreenPosition);

	if (bIsOnScreen)
	{
		float Scale = UWidgetLayoutLibrary::GetViewportScale(this);

		ScreenPosition /= Scale;

		ParentOverlay->SetRenderTranslation(ScreenPosition);
	}

	// Avoid unnecessary invalidation in Slate
	if (bWasOnScreen != bIsOnScreen)
	{
		ParentOverlay->SetVisibility(bIsOnScreen ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	bWasOnScreen = bIsOnScreen;
}

void USWorldUserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Assumes we are directly added to a CanvasPanel rather than direct to viewport.
	UCanvasPanelSlot* CanvasSlot = static_cast<UCanvasPanelSlot*>(Slot);
	// Might be nullptr during init
	if (CanvasSlot)
	{
		CanvasSlot->SetAlignment(FVector2d(0.5f,1.0f));
		CanvasSlot->SetAutoSize(true);
	}
}

void USWorldUserWidget::AddToRootCanvasPanel(UUserWidget* InNewWidget)
{
	// Grab the 'main hud' which will have a Canvas Panel to use.
	TArray<UUserWidget*> Widgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(InNewWidget, Widgets, UUserWidget::StaticClass(), true);
	// Expect only one main hud, other menus etc. might mess with this and requires slight change (eg. adding a pause menu to the root)
	//check(Widgets.Num() == 1)

	UUserWidget* MainHUD = Widgets[0];
	UCanvasPanel* CanvasPanel = Cast<UCanvasPanel>(MainHUD->GetRootWidget());
	CanvasPanel->AddChild(InNewWidget);
}
