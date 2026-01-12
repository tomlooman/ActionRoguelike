// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"


ARogueAIController::ARogueAIController()
{

}

void ARogueAIController::BeginPlay()
{
	Super::BeginPlay();

	RunBehaviorTree(BehaviorTree);

	FName TargetActor = FName("TargetActor");

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	check(PlayerPawn);

	GetBlackboardComponent()->SetValueAsObject(TargetActor, PlayerPawn);
}
