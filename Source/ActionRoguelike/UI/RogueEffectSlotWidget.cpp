// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueEffectSlotWidget.h"
#include "ActionRoguelike/ActionSystem/RogueActionEffect.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Components/Image.h"

void URogueEffectSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	check(EffectInst);
	EffectInst->GetOwningComponent()->OnActionStopped.AddDynamic(this, &URogueEffectSlotWidget::OnEffectEnded);
	
	// Icons may not be specified yet, the widget should continue using a placeholder
	FSoftObjectPath IconPath = EffectInst->GetIcon().ToSoftObjectPath();
	if (IconPath.IsValid())
	{
		FLoadAssetAsyncDelegate Delegate;
		Delegate.BindUObject(this, &URogueEffectSlotWidget::OnLoadIconCompleted);

		LoadAssetAsync(IconPath.GetAssetPath(), Delegate);
	}
}

void URogueEffectSlotWidget::OnLoadIconCompleted(const FTopLevelAssetPath& AssetPath, UObject* LoadedObject, EAsyncLoadingResult::Type Result)
{
	UMaterialInstanceDynamic* MID = EffectImage->GetDynamicMaterial();
	
	// "Icon" param name must match the param name in the Material asset
	MID->SetTextureParameterValue("Icon", CastChecked<UTexture2D>(LoadedObject));
}

void URogueEffectSlotWidget::OnEffectEnded(URogueActionComponent* OwningComp, URogueAction* Action)
{
	// Only remove if the relevant effect stopped not something else
	if (Action == EffectInst)
	{
		RemoveFromParent();
	}
}
