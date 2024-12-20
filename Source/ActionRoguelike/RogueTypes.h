// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class URogueActionComponent;


UENUM(BlueprintType)
enum class EAttributeModifyType: uint8
{
	AddDelta,

	AddBase,

	OverrideBase,

	Invalid
};
