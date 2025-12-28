// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueMessagingSubsystem.h"



void URogueMessagingSubsystem::AddTagListener(FGameplayTag InTag, FOnMessageReceived InEventHook)
{
	FMessageTagListener& Value = TagListeners.FindOrAdd(InTag);
	Value.Listeners.Add(InEventHook);
}


void URogueMessagingSubsystem::BroadcastTag(FGameplayTag InTag, FInstancedStruct InPayload)
{
	if (FMessageTagListener* Value = TagListeners.Find(InTag))
	{
		for (FOnMessageReceived Delegate : Value->Listeners)
		{
			bool bBound = Delegate.ExecuteIfBound(InTag, InPayload);

			// Needs cleanup if no longer bound
			check(bBound);
		}
	}
}

template <typename FMessageStructType>
void URogueMessagingSubsystem::BroadcastTagNative(FGameplayTag InTag, const FMessageStructType& Message)
{
	FInstancedStruct InstStruct;
	InstStruct.InitializeAs<FMessageStructType>(Message);

	BroadcastTag(InTag, InstStruct);
}