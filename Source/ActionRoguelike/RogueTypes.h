// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class USActionComponent;


UENUM(BlueprintType)
enum class EAttributeModifyType: uint8
{
	AddDelta,

	AddBase,

	OverrideBase,

	Invalid
};
