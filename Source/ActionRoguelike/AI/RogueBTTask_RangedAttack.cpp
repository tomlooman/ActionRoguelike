// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueBTTask_RangedAttack.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Core/RogueGameplayFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueBTTask_RangedAttack)


UDEPRECATED_URogueBTTask_RangedAttack::UDEPRECATED_URogueBTTask_RangedAttack()
{
	TargetActorKey.SelectedKeyName = NAME_TargetActor;
}


EBTNodeResult::Type UDEPRECATED_URogueBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACharacter* MyPawn = CastChecked<ACharacter>(OwnerComp.GetAIOwner()->GetPawn());

	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (TargetActor == nullptr || !URogueGameplayFunctionLibrary::IsAlive(TargetActor))
	{
		return EBTNodeResult::Failed;
	}

	const FVector MuzzleLocation = MyPawn->GetMesh()->GetSocketLocation(MuzzleSocket);
	const FVector Direction = TargetActor->GetActorLocation() - MuzzleLocation;
	FRotator MuzzleRotation = Direction.Rotation();

	// Ignore negative pitch to not hit the floor in front itself
	MuzzleRotation.Pitch += FMath::RandRange(0.0f, MaxBulletSpread);
	MuzzleRotation.Yaw += FMath::RandRange(-MaxBulletSpread, MaxBulletSpread);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Instigator = MyPawn;

	AActor* NewProj = GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, MuzzleRotation, Params);

	return NewProj ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
	
}