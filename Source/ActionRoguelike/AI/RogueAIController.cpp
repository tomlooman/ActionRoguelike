// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueAIController.h"

#include "ActionRoguelike.h"
#include "RogueAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Player/RoguePlayerCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAIController)


ARogueAIController::ARogueAIController()
{
	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
}


void ARogueAIController::BeginPlay()
{
	Super::BeginPlay();

	if (ensureMsgf(BehaviorTree, TEXT("Behavior Tree is nullptr! Please assign BehaviorTree in your AI Controller.")))
	{
		RunBehaviorTree(BehaviorTree);
	}

	// Setup a listener to check black board for target getting set so we can notify (clients) via a UI popup that they were spotted	
	UBlackboardComponent* BBComp = GetBlackboardComponent();
	BBComp->RegisterObserver(BBComp->GetKeyID(NAME_TargetActor), this,
		FOnBlackboardChangeNotification::CreateUObject(this, &ThisClass::OnTargetActorChanged));
}

void ARogueAIController::PreRegisterAllComponents()
{
	Super::PreRegisterAllComponents();
	
	// Set default for all AI - must be set before the Pawn is registered as a sense, otherwise we register the wrong team ID with perception system
	SetGenericTeamId(FGenericTeamId(TEAM_ID_BOTS));
}


EBlackboardNotificationResult ARogueAIController::OnTargetActorChanged(const UBlackboardComponent& Comp, FBlackboard::FKey KeyID)
{
	// Filter to only players as something we want to notify
	ARoguePlayerCharacter* TargetPlayer = Cast<ARoguePlayerCharacter>(Comp.GetValueAsObject(NAME_TargetActor));
	if (TargetPlayer)
	{
		// This will be running on the host/server as AI controllers only exist there, therefor we run a Client RPC on the character
		// so the local player will see the notification
		TargetPlayer->ClientOnSeenBy(CastChecked<ARogueAICharacter>(GetPawn()));

		// In our current behavior, we can remove the listener after first successful trigger
		//return EBlackboardNotificationResult::RemoveObserver;
	}

	return EBlackboardNotificationResult::ContinueObserving;
}
