// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueBTService_CheckRangeTo.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void URogueBTService_CheckRangeTo::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	check(BBComp);

	AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (TargetActor)
	{
		AAIController* Controller = OwnerComp.GetAIOwner();
		
		APawn* OwningPawn = Controller->GetPawn();
		check(OwningPawn);

		float DistanceTo = FVector::Dist(TargetActor->GetActorLocation(), OwningPawn->GetActorLocation());

		bool bWithinRange = DistanceTo < MaxAttackRange;

		bool bHasLOS = Controller->LineOfSightTo(TargetActor);

		BBComp->SetValueAsBool(WithinRangeKey.SelectedKeyName, (bWithinRange && bHasLOS));
	}
}
