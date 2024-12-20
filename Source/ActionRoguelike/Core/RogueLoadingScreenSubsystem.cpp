// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/RogueLoadingScreenSubsystem.h"
#include "MoviePlayer.h"


void URogueLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Show simple loading screen when transitioning between maps
	if (!IsRunningDedicatedServer())
	{
		FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &URogueLoadingScreenSubsystem::BeginLoadingScreen);
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &URogueLoadingScreenSubsystem::EndLoadingScreen);
	}
}


void URogueLoadingScreenSubsystem::BeginLoadingScreen(const FString& MapName)
{
	FLoadingScreenAttributes LoadingScreen;
	//LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
	//LoadingScreen.bWaitForManualStop = true;
	LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}


void URogueLoadingScreenSubsystem::EndLoadingScreen(UWorld* InLoadedWorld)
{
	
}
