// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueDamageNumberWidget.h"

#include "Components/TextBlock.h"


void URogueDamageNumberWidget::SetDamageAmount(float InNewAmount)
{
	// note: should enable SimpleTextMode on the TextBlock in UMG for performance
	
	DamageTextBlock->SetText(FText::AsNumber(InNewAmount));
}
