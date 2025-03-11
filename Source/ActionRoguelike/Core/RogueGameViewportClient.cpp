// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueGameViewportClient.h"

#include "SignificanceManager.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueGameViewportClient)

void URogueGameViewportClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (USignificanceManager* SignificanceManager = USignificanceManager::Get(World))
	{
		// Iterate all playercontrollers, for hosting player this means all clients too
		// Helps keeping significance in sync between each client and server
		// ** Main desync issue right now could be WasRecentlyRendered which isn't going to be replicated on the host **
		ViewpointsArray.Empty(GetWorld()->GetNumPlayerControllers());
		for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			APlayerController* PlayerController = Iterator->Get();

			FVector ViewLocation;
			FRotator ViewRotation;
			PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

			ViewpointsArray.Add(FTransform(ViewRotation, ViewLocation, FVector::OneVector));
		}
		
		SignificanceManager->Update(ViewpointsArray);
	}
}
