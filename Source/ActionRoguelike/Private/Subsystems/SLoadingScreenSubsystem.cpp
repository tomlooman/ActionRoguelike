// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/SLoadingScreenSubsystem.h"
#include "MoviePlayer.h"


void USLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Show simple loading screen when transitioning between maps
	if (!IsRunningDedicatedServer())
	{
		FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &USLoadingScreenSubsystem::BeginLoadingScreen);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &USLoadingScreenSubsystem::EndLoadingScreen);
	}
}


void USLoadingScreenSubsystem::BeginLoadingScreen(const FString& MapName)
{
	FLoadingScreenAttributes LoadingScreen;
	//LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
	//LoadingScreen.bWaitForManualStop = true;
	LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}


void USLoadingScreenSubsystem::EndLoadingScreen(UWorld* InLoadedWorld)
{
	
}
