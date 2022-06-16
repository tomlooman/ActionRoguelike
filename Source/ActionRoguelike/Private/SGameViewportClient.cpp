// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameViewportClient.h"

#include "SignificanceManager.h"
#include "Kismet/GameplayStatics.h"

void USGameViewportClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (USignificanceManager* SignificanceManager = FSignificanceManagerModule::Get(World))
	{
		// Update once per frame, using only Player 0
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			FVector ViewLocation;
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

			// Viewpoints
			TArray<FTransform> TransformArray;
			TransformArray.Emplace(ViewRotation, ViewLocation, FVector::OneVector);

			SignificanceManager->Update(TArrayView<FTransform>(TransformArray));
		}
	}
}
