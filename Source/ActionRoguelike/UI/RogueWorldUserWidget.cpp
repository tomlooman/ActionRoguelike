// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueWorldUserWidget.h"

#include "RogueHUD.h"
#include "RogueMainHUDWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void URogueWorldUserWidget::AddToRootCanvas()
{
	ARogueHUD* HUD = Cast<ARogueHUD>(GetOwningPlayer()->GetHUD());
	
	if (URogueMainHUDWidget* MainWidget = HUD->GetMainHUD())
	{
		MainWidget->MainCanvasPanel->AddChild(this);
	}
	
	// Center the Overlay
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	CanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
}

void URogueWorldUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	FVector2D ScreenPosition;
	UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), OwningComponent->GetComponentLocation(), ScreenPosition);
	
	float UIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	ScreenPosition /= UIScale;
	
	ParentOverlay->SetRenderTranslation(ScreenPosition);
}
