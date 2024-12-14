// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAction_MinionRangedAttack.h"

#include "ActionRoguelike.h"
#include "SActionComponent.h"
#include "SAttributeComponent.h"
#include "AI/SAICharacter.h"
#include "GameFramework/Character.h"

void USAction_MinionRangedAttack::StartAction_Implementation(AActor* Instigator)
{
	ASAICharacter* MyPawn = CastChecked<ASAICharacter>(GetOwningComponent()->GetOwner());

	AActor* TargetActor = MyPawn->GetTargetActor();
	if (TargetActor == nullptr)
	{
		return;
	}

	if (!USAttributeComponent::IsActorAlive(TargetActor))
	{
		return;
	}

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
}
