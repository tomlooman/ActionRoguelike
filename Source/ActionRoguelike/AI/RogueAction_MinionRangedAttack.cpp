// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueAction_MinionRangedAttack.h"

#include "ActionRoguelike.h"
#include "ActionSystem/RogueActionComponent.h"
#include "AI/RogueAICharacter.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Projectiles/RogueProjectile.h"
#include "Projectiles/RogueProjectilesSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAction_MinionRangedAttack)


void URogueAction_MinionRangedAttack::StartAction_Implementation(AActor* Instigator)
{
	ARogueAICharacter* MyPawn = CastChecked<ARogueAICharacter>(GetOwningComponent()->GetOwner());

	Super::StartAction_Implementation(Instigator);

	// Handle the cosmetic parts, push to all clients
	MyPawn->MulticastPlayAttackFX();

	const FVector MuzzleLocation = MyPawn->GetMesh()->GetSocketLocation(MeshSockets::RightHandMuzzle);
	// using TargetLocation instead of ActorLocation we *could* retrieve 'better' aiming location if implemented
	AActor* TargetActor = MyPawn->GetTargetActor();
	check(TargetActor);

	const ARogueProjectile* DefaultProj = Cast<ARogueProjectile>(ProjectileClass->GetDefaultObject());

	// Est. time to target based on distance-to and projectile assumed velocity
	const float ProjectileSpeed = DefaultProj->GetDefaultSpeed();
	float DistanceToTarget = (TargetActor->GetTargetLocation() - MuzzleLocation).Size();
	float TravelTime = DistanceToTarget / ProjectileSpeed;
	

	// Optionally set very small gravity if projectile has scale == 0, otherwise func will use the world default gravity in the calc below when passing in 0.f as override
	const float TinyGravity = 0.01f;
	const float ActualProjectileGravity = GetWorld()->GetGravityZ() * DefaultProj->GetGravityScale();
	const float GravityOverride = FMath::IsNearlyZero(DefaultProj->GetGravityScale()) ? TinyGravity : ActualProjectileGravity;

	FVector LaunchVelocity;
	UGameplayStatics::SuggestProjectileVelocity_MovingTarget(this, LaunchVelocity, MyPawn->GetActorLocation(), TargetActor, FVector::ZeroVector,
		GravityOverride, TravelTime, EDrawDebugTrace::None);
	// Override muzzle...with our approximate launch direction, based on an approx. desired flight time, can still be poorly targetted but thats OK for AI!
	FRotator MuzzleRotation = LaunchVelocity.Rotation();

	// to further improve the feel of enemy targeting, we could use an averaged velocity (taking past 1 second of velocities from TargetActor) in the function above, this adds some stickiness to the velocity on the player's movement
	// I expect this will make the enemy feel like its targetting us slightly better when we just started moving in a certain direction (it'll use a low velocity instead of the max)
	// same for move direction changes, it'll stick to targeting the 'recent' velocity direction instead so that their reflexes don't feel faster than a player could do.
	// Note: We need to replace the function SuggestProjectileVelocity_MovingTarget with a variant that doesn't grab TargetActor->GetVelocity but instead our own averaged velocity over the past 1 second or so.

	// Ignore negative pitch to not hit the floor in front itself
	MuzzleRotation.Pitch += FMath::RandRange(0.0f, MaxBulletSpread);
	MuzzleRotation.Yaw += FMath::RandRange(-MaxBulletSpread, MaxBulletSpread);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Instigator = MyPawn;

	// Disabled for testing data oriented projectiles below
	//GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, MuzzleRotation, Params);

	// WIP for data oriented projectiles
	URogueProjectilesSubsystem* Subsystem = GetWorld()->GetSubsystem<URogueProjectilesSubsystem>();
	Subsystem->CreateProjectile(MuzzleLocation, MuzzleRotation.Vector(), ProjectileConfig, MyPawn);

	StopAction(Instigator);
}

bool URogueAction_MinionRangedAttack::CanStart_Implementation(AActor* Instigator)
{
	if (Super::CanStart_Implementation(Instigator))
	{
		const ARogueAICharacter* MyPawn = CastChecked<ARogueAICharacter>(GetOwningComponent()->GetOwner());
		AActor* TargetActor = MyPawn->GetTargetActor();
	
		return TargetActor && URogueGameplayFunctionLibrary::IsAlive(TargetActor);
	}

	return false;
}
