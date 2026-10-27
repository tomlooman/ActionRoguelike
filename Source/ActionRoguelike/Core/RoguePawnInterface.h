// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RoguePawnInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(meta=(CannotImplementInterfaceInBlueprint))
class URoguePawnInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ACTIONROGUELIKE_API IRoguePawnInterface
{
	GENERATED_BODY()
	
	// C++ only interface by design, see CannotImplementInterfaceInBlueprint

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	virtual bool IsAlive() const = 0;
};
