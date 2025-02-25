// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RogueSignificanceInterface.generated.h"

// Marked not blueprintable, this lets us use it more easily and hopefully more performant. All usage simply need a base class in C++
UINTERFACE(MinimalAPI/*, meta=(CannotImplementInterfaceInBlueprint)*/)
class URogueSignificanceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ACTIONROGUELIKE_API IRogueSignificanceInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void SignificanceLODChanged(int32 NewLOD) = 0;
	
	
	UFUNCTION(BlueprintNativeEvent, Category = "Significance")
	bool GetSignificanceLocation(FTransform Viewpoint, FVector& OutLocation);

	UFUNCTION(BlueprintNativeEvent, Category = "Significance")
	bool CalcSignificanceOverride(float& OutSignificance);
	
};
