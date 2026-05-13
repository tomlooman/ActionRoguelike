// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/RogueWorldUserWidget.h"

#include "ActionRoguelike.h"
#include "RogueHUD.h"
#include "RogueMainHUDWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/SizeBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueWorldUserWidget)



void URogueWorldUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
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

void URogueWorldUserWidget::NativeConstruct()
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

void URogueWorldUserWidget::AddToRootCanvasPanel()
{
	ARogueHUD* HUD = Cast<ARogueHUD>(GetOwningPlayer()->GetHUD());
	check(HUD);
	
	// We might be testing without a HUD Widget...
	if (URogueMainHUDWidget* Widget = HUD->GetMainHUD())
	{
		Widget->MainCanvasPanel->AddChild(this);
	}
}

void URogueWorldUserWidget::RemoveFromParent()
{
	Super::RemoveFromParent();

	// @todo: WidgetPool.Release(this)
}

void URogueWorldUserWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
}
