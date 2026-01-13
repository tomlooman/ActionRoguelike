// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueBTTask_RangedAttack.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Projectiles/RogueProjectile.h"


EBTNodeResult::Type URogueBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ACharacter* Pawn = Cast<ACharacter>(OwnerComp.GetAIOwner()->GetPawn());
	check(Pawn);

	FVector SpawnLocation = Pawn->GetMesh()->GetSocketLocation(MuzzleSocketName);

	AActor* TargetActor = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!IsValid(TargetActor))
	{
		return EBTNodeResult::Failed;
	}
	
	FVector Direction = TargetActor->GetActorLocation() - SpawnLocation;

	FRotator SpawnRotation = Direction.Rotation();

	SpawnRotation.Pitch += FMath::FRandRange(0.0f, MaxBulletSpread);
	SpawnRotation.Yaw += FMath::FRandRange(-MaxBulletSpread, MaxBulletSpread);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = Pawn;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* NewProj = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	
	return NewProj ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
