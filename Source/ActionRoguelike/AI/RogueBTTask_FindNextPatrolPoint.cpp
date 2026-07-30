// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueBTTask_FindNextPatrolPoint.h"

#include "ActionRoguelike.h"
#include "AIController.h"
#include "EngineUtils.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "World/RoguePatrolPoint.h"


EBTNodeResult::Type URogueBTTask_FindNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{	
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();

	ARoguePatrolPoint* CurrentPatrolPoint = Cast<ARoguePatrolPoint>(BBComp->GetValueAsObject(CurrentPatrolPointKey.SelectedKeyName));
	if (!IsValid(CurrentPatrolPoint))
	{
		CurrentPatrolPoint = FindNearestPatrolActor(OwnerComp);
		
		// If we still didn't find anything, we can't do much more...
		if (CurrentPatrolPoint == nullptr)
		{
			UE_LOG(LogGame, Warning, TEXT("No PatrolPoint available for %s"), *GetNameSafe(OwnerComp.GetOwner()));
			return EBTNodeResult::Failed;
		}
	}

	TArray<ARoguePatrolPoint*> Connections = CurrentPatrolPoint->GetConnections();
	if (Connections.Num() == 0)
	{
		UE_LOG(LogGame, Warning, TEXT("PatrolPoint %s has no connections available for %s"), 
			*GetNameSafe(CurrentPatrolPoint), *GetNameSafe(OwnerComp.GetOwner()));
		return EBTNodeResult::Failed;
	}

	ARoguePatrolPoint* OldPatrolPoint = Cast<ARoguePatrolPoint>(BBComp->GetValueAsObject(PrevPatrolPointKey.SelectedKeyName));
	// Store the "current" as the new previous patrol
	BBComp->SetValueAsObject(PrevPatrolPointKey.SelectedKeyName, CurrentPatrolPoint);
	
	if (Connections.Num() > 1)
	{
		// Remove previous point to avoid re-picking
		Connections.RemoveSingle(OldPatrolPoint);
	}
	
	// Randomly pick new patrol point
	const int32 RandomIndex = FMath::RandRange(0, Connections.Num()-1);
	ARoguePatrolPoint* NewPoint = Connections[RandomIndex];
	BBComp->SetValueAsObject(CurrentPatrolPointKey.SelectedKeyName, NewPoint);
	
	return EBTNodeResult::Succeeded;
}

ARoguePatrolPoint* URogueBTTask_FindNextPatrolPoint::FindNearestPatrolActor(UBehaviorTreeComponent& OwnerComp)
{
	ARoguePatrolPoint* NearestPatrolActor = nullptr;
	float NearestDistSqrd = MAX_FLT;
	
	// Assume we have a Pawn at all which is true for our current implementation, otherwise 
	// it may need to be called later in the init sequence
	APawn* AIPawn = OwnerComp.GetAIOwner()->GetPawn();
	check(AIPawn);
	FVector PawnLocation = AIPawn->GetActorLocation();
		
	// Find nearest, iterating ALL patrol points.
	for (ARoguePatrolPoint* PatrolActor : TActorRange<ARoguePatrolPoint>(GetWorld()))
	{
		float DistTo = FVector::DistSquared(PawnLocation, PatrolActor->GetActorLocation());
		if (DistTo < NearestDistSqrd)
		{
			NearestDistSqrd = DistTo;
			NearestPatrolActor = PatrolActor;
		}
	}

	return NearestPatrolActor;
}