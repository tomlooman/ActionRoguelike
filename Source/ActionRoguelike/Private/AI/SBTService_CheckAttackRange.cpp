// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SBTService_CheckAttackRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SBTService_CheckAttackRange)


USBTService_CheckAttackRange::USBTService_CheckAttackRange()
{
	MaxAttackRange = 2000.f;

	TargetActorKey.SelectedKeyName = "TargetActor";
}


void USBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Check distance between ai pawn and target actor
	UBlackboardComponent* BlackBoardComp = OwnerComp.GetBlackboardComponent();
	check(BlackBoardComp);
	
	AActor* TargetActor = Cast<AActor>(BlackBoardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (TargetActor)
	{
		AAIController* MyController = OwnerComp.GetAIOwner();
		check(MyController);
		
		const float DistanceTo = FVector::Distance(TargetActor->GetActorLocation(), MyController->GetPawn()->GetActorLocation());
		const bool bWithinRange = DistanceTo < MaxAttackRange;

		bool bHasLOS = false;
		if (bWithinRange)
		{
			bHasLOS = MyController->LineOfSightTo(TargetActor);
		}

		BlackBoardComp->SetValueAsBool(AttackRangeKey.SelectedKeyName, (bWithinRange && bHasLOS));
	}
}
