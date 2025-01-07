// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueAction_MinionRangedAttack.h"

#include "ActionRoguelike.h"
#include "ActionSystem/RogueActionComponent.h"
#include "AI/RogueAICharacter.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAction_MinionRangedAttack)


void URogueAction_MinionRangedAttack::StartAction_Implementation(AActor* Instigator)
{
	ARogueAICharacter* MyPawn = CastChecked<ARogueAICharacter>(GetOwningComponent()->GetOwner());

	AActor* TargetActor = MyPawn->GetTargetActor();
	if (TargetActor == nullptr || !URogueGameplayFunctionLibrary::IsAlive(TargetActor))
	{
		return;
	}

	// Handle the cosmetic parts, push to all clients
	MyPawn->MulticastPlayAttackFX();

	const FVector MuzzleLocation = MyPawn->GetMesh()->GetSocketLocation(MeshSockets::Muzzle);
	// using TargetLocation instead of ActorLocation we *could* retrieve 'better' aiming location if implemented
	const FVector Direction = TargetActor->GetTargetLocation() - MuzzleLocation;
	FRotator MuzzleRotation = Direction.Rotation();

	// Ignore negative pitch to not hit the floor in front itself
	MuzzleRotation.Pitch += FMath::RandRange(0.0f, MaxBulletSpread);
	MuzzleRotation.Yaw += FMath::RandRange(-MaxBulletSpread, MaxBulletSpread);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Instigator = MyPawn;

	GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, MuzzleRotation, Params);

	// @todo: no StopAction required??
}
