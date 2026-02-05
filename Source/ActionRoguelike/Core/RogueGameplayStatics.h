// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RogueGameplayStatics.generated.h"

class URogueActionSystemComponent;
/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueGameplayStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	static bool IsFullHealth(URogueActionSystemComponent* ActionComp);
};
