// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RogueMessagingSubsystem.generated.h"


#define USE_TAGMESSAGING_SYSTEM 0

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMessageReceived, FGameplayTag, InMessageTag, FInstancedStruct, Payload);

/*
 * Wrapper - to support array inside TMap
 */
USTRUCT()
struct FMessageTagListener
{
	GENERATED_BODY()

	//FGameplayTag Tag;
	TArray<FOnMessageReceived> Listeners;
};


USTRUCT(BlueprintType)
struct FPayLoadTestMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Credits = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* VictimActor = nullptr;
};

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueMessagingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
	void AddTagListener(FGameplayTag InTag, FOnMessageReceived InEventHook);

	// @todo: RemoveTagListener()

	/* 
	 * Broadcast Message using whatever struct payload we may need.
	 * Look at UBlueprintInstancedStructLibrary::MakeInstancedStruct to implement this with CustomThunk to simplify its usage in blueprint.
	 */
	UFUNCTION(BlueprintCallable)
	void BroadcastTag(FGameplayTag InTag, FInstancedStruct InPayload);

	template <typename FMessageStructType>
	void BroadcastTagNative(FGameplayTag InTag, const FMessageStructType& Message);
	//void BroadcastTagNative(FGameplayTag InTag, const FPayLoadTestMessage& Message);

protected:
	
	TMap<FGameplayTag, FMessageTagListener> TagListeners;
};

