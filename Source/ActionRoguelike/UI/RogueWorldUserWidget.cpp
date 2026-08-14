// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueWorldUserWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void URogueWorldUserWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	FVector2D ScreenPosition;
	UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), OwningComponent->GetComponentLocation(), ScreenPosition);
	
	float UIScale = UWidgetLayoutLibrary::GetViewportScale(this);
	ScreenPosition /= UIScale;
	
	ParentOverlay->SetRenderTranslation(ScreenPosition);
}
