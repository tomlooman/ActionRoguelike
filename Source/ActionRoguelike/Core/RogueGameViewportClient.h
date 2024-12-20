// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "RogueGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
	
	virtual void Tick(float DeltaTime) override;
	
};
