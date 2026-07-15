// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAIController.h"

#include "RogueGameTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"


ARogueAIController::ARogueAIController()
{
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
}

void ARogueAIController::PreRegisterAllComponents()
{
	Super::PreRegisterAllComponents();
	
	// Needs to be super early before pawn is registered or perception system has the wrong teamID
	SetGenericTeamId(FGenericTeamId(TEAM_ID_BOTS));
}

void ARogueAIController::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(BehaviorTree);

	/*FName TargetActor = FName("TargetActor");

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	check(PlayerPawn);

	GetBlackboardComponent()->SetValueAsObject(TargetActor, PlayerPawn);*/
}
