// Fill out your copyright notice in the Description page of Project Settings.


#include "SAction_ProjectileAttack.h"
#include "ActionRoguelike.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "Subsystems/SActorPoolingSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SAction_ProjectileAttack)


USAction_ProjectileAttack::USAction_ProjectileAttack()
{
	HandSocketName = "Muzzle_01";
	AttackAnimDelay = 0.2f;

	SweepRadius = 20.0f;
	SweepDistanceFallback = 5000;
}


void USAction_ProjectileAttack::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);

	ACharacter* Character = CastChecked<ACharacter>(Instigator);
	Character->PlayAnimMontage(AttackAnim);

	// Auto-released particle pooling
	UGameplayStatics::SpawnEmitterAttached(CastingEffect, Character->GetMesh(), HandSocketName, FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget, true, EPSCPoolMethod::AutoRelease);

	UGameplayStatics::SpawnSoundAttached(CastingSound, Character->GetMesh());

	if (Character->HasAuthority())
	{
		FTimerHandle TimerHandle_AttackDelay;
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "AttackDelay_Elapsed", Character);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, Delegate, AttackAnimDelay, false);
	}
}


void USAction_ProjectileAttack::AttackDelay_Elapsed(ACharacter* InstigatorCharacter)
{
	// Blueprint has not been properly configured yet if this fails
	if (ensureAlways(ProjectileClass))
	{
		FVector HandLocation = InstigatorCharacter->GetMesh()->GetSocketLocation(HandSocketName);

		// We trace against the environment first to find whats under the player crosshair.
		// We use the hit location to adjust the projectile launch direction so it will hit what is under the crosshair rather than shoot straight forward from the player hands.

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Instigator = InstigatorCharacter;

		FCollisionShape Shape;
		Shape.SetSphere(SweepRadius);

		// Ignore Player
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(InstigatorCharacter);

		FVector TraceDirection = InstigatorCharacter->GetControlRotation().Vector();

		// Add sweep radius onto start to avoid the sphere clipping into floor/walls the camera is directly against.
		const FVector TraceStart = InstigatorCharacter->GetPawnViewLocation() + (TraceDirection * SweepRadius);
		// endpoint far into the look-at distance (not too far, still adjust somewhat towards crosshair on a miss)
		const FVector TraceEnd = TraceStart + (TraceDirection * SweepDistanceFallback);
		FVector AdjustedTraceEnd = TraceEnd;

		TArray<FHitResult> Hits;
		// Enemies are using Overlap response to Projectiles, we cant look for single Blocking hits and instead look for all overlaps and filter after
		if (GetWorld()->SweepMultiByChannel(Hits, TraceStart, TraceEnd, FQuat::Identity, COLLISION_PROJECTILE, Shape, Params))
		{
			// Overwrite trace end with impact point in world
			// First entry must exist and first entry will be first overlap or block
			// Could filter further, eg. ignoring friendly players between us and the enemy
			AdjustedTraceEnd = Hits[0].ImpactPoint;
		}

// Removes debug code from shipping builds		
#if !UE_BUILD_SHIPPING
		//const float DrawDuration = 5.0f;
		// Start
		//DrawDebugPoint(GetWorld(), TraceStart, 8, FColor::Green, false, DrawDuration);
		// End - possibly adjusted based on hit
		//DrawDebugPoint(GetWorld(), AdjustedTraceEnd, 8, FColor::Green, false, DrawDuration);
		//DrawDebugLine(GetWorld(), TraceStart, AdjustedTraceEnd, FColor::Green, false, DrawDuration);
		// End - Original
		//DrawDebugPoint(GetWorld(), TraceEnd, 8, FColor::Red, false, DrawDuration);
		//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, DrawDuration);
#endif

		// find new direction/rotation from Hand pointing to impact point in world.
		FRotator ProjRotation = (AdjustedTraceEnd - HandLocation).Rotation();

		FTransform SpawnTM = FTransform(ProjRotation, HandLocation);

		// Standard spawning, replaced by pooling system below
		//GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTM, SpawnParams);

		// re-use a pooled actor instead of always spawning new Actors
		USActorPoolingSubsystem::AcquireFromPool(this, ProjectileClass, SpawnTM, SpawnParams);
	}

	StopAction(InstigatorCharacter);
}
