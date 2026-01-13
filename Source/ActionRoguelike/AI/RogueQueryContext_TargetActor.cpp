// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueQueryContext_TargetActor.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"


void URogueQueryContext_TargetActor::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// Querier should be our AI Controller
	UObject* QuerierObject = QueryInstance.Owner.Get();

	AAIController* Controller = Cast<AAIController>(QuerierObject);
	// Querier Could be misconfigured by user inside EQS @todo: impl. log warning
	if (ensure(Controller))
	{
		AActor* TargetActor = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(TargetActorName));

		UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);		
	}
}
