// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueQueryContext_TargetActor.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"


void URogueQueryContext_TargetActor::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// Expect Pawn/Character as instigator/owner
	APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
	if (ensure(QuerierPawn))
	{
		AAIController* Controller = Cast<AAIController>(QuerierPawn->GetController());
		check(Controller);
		
		AActor* TargetActor = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(TargetActorName));

		UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);		
	}
}
