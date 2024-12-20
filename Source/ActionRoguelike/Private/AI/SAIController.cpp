// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAIController.h"

#include "ActionRoguelike.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SAIController)


ASAIController::ASAIController()
{
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
}


void ASAIController::BeginPlay()
{
	Super::BeginPlay();
	
	// Set default for for all AI
	SetGenericTeamId(FGenericTeamId(TEAM_ID_BOTS));

	if (ensureMsgf(BehaviorTree, TEXT("Behavior Tree is nullptr! Please assign BehaviorTree in your AI Controller.")))
	{
		RunBehaviorTree(BehaviorTree);
	}
}
