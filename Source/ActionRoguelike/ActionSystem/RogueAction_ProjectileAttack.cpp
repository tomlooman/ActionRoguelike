// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAction_ProjectileAttack.h"
#include "ActionRoguelike.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Character.h"
#include "Performance/RogueActorPoolingSubsystem.h"
#include "Player/RoguePlayerCharacter.h"
#include "Projectiles/RogueProjectilesSubsystem.h"
#include "RogueActionComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAction_ProjectileAttack)




void URogueAction_ProjectileAttack::StartAction_Implementation(AActor* Instigator)
{
	Super::StartAction_Implementation(Instigator);
	
	// Other clients should not run this logic ever
	check(GetOwningActor()->GetLocalRole() != ROLE_SimulatedProxy);

	GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, this, &ThisClass::AttackDelay_Elapsed,
		FMath::Max(AttackAnimDelay, SMALL_NUMBER), false);
}

void URogueAction_ProjectileAttack::StopAction_Implementation(AActor* Instigator)
{
	const FProjectilePredictionKey CancelledKey = ServerPredictionKey;
	ServerPredictionKey = {};
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AttackDelay);
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_PredictionTimeout);
	if (CancelledKey.IsValid() && !bProjectileReleased)
	{
		GetOwningComponent()->RejectProjectilePrediction(CancelledKey, EProjectileOutcome::Cancelled);
	}
	Super::StopAction_Implementation(Instigator);
}


void URogueAction_ProjectileAttack::BroadcastActionCosmetics()
{
#if USE_DATA_ORIENTED_PROJECTILES
	GetOwningComponent()->MulticastPlayProjectileCosmetics(this, ServerPredictionKey);
#else
	Super::BroadcastActionCosmetics();
#endif
}


void URogueAction_ProjectileAttack::PlayActionCosmetics()
{
	Super::PlayActionCosmetics();
	
	ARoguePlayerCharacter* OwningCharacter = Cast<ARoguePlayerCharacter>(GetOwningActor());
	
	OwningCharacter->PlayAnimMontage(AttackAnim);

	// Auto-released particle pooling
	UNiagaraFunctionLibrary::SpawnSystemAttached(CastingEffect, OwningCharacter->GetMesh(), HandSocketName, FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget, true, true, ENCPoolMethod::AutoRelease);

	//UGameplayStatics::SpawnSoundAttached(CastingSound, Character->GetMesh());
	// Alternative to spawning fresh instances for short-lived attacks every time (via SpawnSoundAttached above)
	// we use a single audio component on the player, which uses AutoManageAttachment to detach itself when not active
	OwningCharacter->PlayAttackSound(CastingSound);
}


void URogueAction_ProjectileAttack::AttackDelay_Elapsed()
{
#if USE_DATA_ORIENTED_PROJECTILES
	if (ServerPredictionKey.IsValid())
	{
		bReadyToRelease = true;
		ReleasePredictedProjectile();
		return;
	}
#endif
	ARoguePlayerCharacter* OwningCharacter = Cast<ARoguePlayerCharacter>(GetOwningActor());
	FVector HandLocation;
	FVector Direction;
	if (IsValid(OwningCharacter) && CalculateProjectileLaunch(OwningCharacter->GetControlRotation().Vector(), HandLocation, Direction))
	{
#if USE_DATA_ORIENTED_PROJECTILES
		check(ProjectileConfig);
		GetWorld()->GetSubsystem<URogueProjectilesSubsystem>()->CreateProjectile(HandLocation, Direction, ProjectileConfig, OwningCharacter);
#else
		if (ensureAlways(ProjectileClass))
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			SpawnParams.Instigator = OwningCharacter;
			FTransform SpawnTM(Direction.Rotation(), HandLocation);
			URogueActorPoolingSubsystem* Pooler = GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
			Pooler->AcquireFromPool<AActor>(ProjectileClass, SpawnTM, SpawnParams);
		}
#endif
	}
	StopAction(OwningCharacter);
}


bool URogueAction_ProjectileAttack::CalculateProjectileLaunch(FVector AimDirection, FVector& HandLocation, FVector& LaunchDirection) const
{
	ARoguePlayerCharacter* OwningCharacter = Cast<ARoguePlayerCharacter>(GetOwningActor());
	if (!IsValid(OwningCharacter) || !OwningCharacter->IsAlive() || AimDirection.ContainsNaN() || AimDirection.IsNearlyZero())
	{
		return false;
	}
	HandLocation = OwningCharacter->GetMesh()->GetSocketLocation(HandSocketName);

	FCollisionShape Shape;
	Shape.SetSphere(SweepRadius);

	// Ignore Player
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwningCharacter);

	const FVector TraceDirection = AimDirection.GetSafeNormal();

	// Add sweep radius onto start to avoid the sphere clipping into floor/walls the camera is directly against.
	const FVector TraceStart = OwningCharacter->GetPawnViewLocation() + (TraceDirection * SweepRadius);
	// endpoint far into the look-at distance (not too far, still adjust somewhat towards crosshair on a miss)
	const FVector TraceEnd = TraceStart + (TraceDirection * SweepDistanceFallback);
	FVector AdjustedTraceEnd = TraceEnd;

	TArray<FHitResult> Hits;
	// Enemies are using Overlap response to Projectiles, we cant look for single Blocking hits and instead look for all overlaps and filter after
	GetWorld()->SweepMultiByChannel(Hits, TraceStart, TraceEnd, FQuat::Identity, COLLISION_PROJECTILE, Shape, Params);
	if (Hits.Num() > 0)
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
	LaunchDirection = (AdjustedTraceEnd - HandLocation).GetSafeNormal();
	return !LaunchDirection.IsNearlyZero();

}


bool URogueAction_ProjectileAttack::CanPredictProjectile() const
{
	const ARoguePlayerCharacter* Character = Cast<ARoguePlayerCharacter>(GetOwningActor());
	return IsValid(Character) && Character->IsAlive() && !LocalPredictionKey.IsValid() && GetWorld()->GetTimeSeconds() >= NextLocalAttackTime
		&& CooldownTimeRemaining() <= 0 && !GetOwningComponent()->GetActiveTags().HasAny(BlockedTags);
}


void URogueAction_ProjectileAttack::StartPrediction(FProjectilePredictionKey Key)
{
	check(ProjectileConfig);
	LocalPredictionKey = Key;
	PlayActionCosmetics();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_AttackDelay, this, &ThisClass::PredictionDelay_Elapsed,
		FMath::Max(AttackAnimDelay, SMALL_NUMBER), false);
}


void URogueAction_ProjectileAttack::CancelPrediction(FProjectilePredictionKey Key)
{
	if (!Key.IsValid() || LocalPredictionKey == Key)
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AttackDelay);
		LocalPredictionKey = {};
		NextLocalAttackTime = FMath::Max(NextLocalAttackTime, GetWorld()->GetTimeSeconds() + FMath::Max(0.05f, CooldownTime));
	}
}


void URogueAction_ProjectileAttack::PredictionDelay_Elapsed()
{
	const FProjectilePredictionKey Key = LocalPredictionKey;
	LocalPredictionKey = {};
	NextLocalAttackTime = GetWorld()->GetTimeSeconds() + CooldownTime;
	ARoguePlayerCharacter* Character = Cast<ARoguePlayerCharacter>(GetOwningActor());
	if (!Key.IsValid() || !IsValid(Character))
	{
		return;
	}
	const FVector Aim = Character->GetControlRotation().Vector();
	FVector Position;
	FVector Direction;
	if (CalculateProjectileLaunch(Aim, Position, Direction))
	{
		URogueActionComponent* Component = GetOwningComponent();
		GetWorld()->GetSubsystem<URogueProjectilesSubsystem>()->PredictProjectile(Position, Direction, ProjectileConfig, Character, Key, Component);
		Component->ServerReleaseProjectile(Key, Aim);
	}
	else
	{
		GetOwningComponent()->StopActionByName(Character, GetActivationTag());
	}
}


void URogueAction_ProjectileAttack::StartServerPrediction(FProjectilePredictionKey Key)
{
	check(GetOwningActor()->HasAuthority());
	check(ProjectileConfig);
	ServerPredictionKey = Key;
	bHasPredictedAim = false;
	bReadyToRelease = false;
	bProjectileReleased = false;
	StartAction(GetOwningActor());
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_PredictionTimeout, this, &ThisClass::PredictionTimeout_Elapsed,
		FMath::Max(AttackAnimDelay, 0.0f) + 2.0f, false);
}


void URogueAction_ProjectileAttack::SubmitPredictedAim(FProjectilePredictionKey Key, FVector AimDirection)
{
	check(GetOwningActor()->HasAuthority());
	if (Key != ServerPredictionKey || bHasPredictedAim || bProjectileReleased || !IsRunning())
	{
		return;
	}
	if (AimDirection.ContainsNaN() || AimDirection.IsNearlyZero())
	{
		GetOwningComponent()->RejectProjectilePrediction(Key, EProjectileOutcome::Rejected);
		ServerPredictionKey = {};
		StopAction(GetOwningActor());
		return;
	}
	PendingAimDirection = AimDirection.GetSafeNormal();
	bHasPredictedAim = true;
	ReleasePredictedProjectile();
}


void URogueAction_ProjectileAttack::ReleasePredictedProjectile()
{
	if (!bReadyToRelease || !bHasPredictedAim || bProjectileReleased)
	{
		return;
	}
	FVector Position;
	FVector Direction;
	if (CalculateProjectileLaunch(PendingAimDirection, Position, Direction))
	{
		bProjectileReleased = true;
		GetWorld()->GetSubsystem<URogueProjectilesSubsystem>()->LaunchProjectile(Position, Direction, ProjectileConfig,
			GetOwningActor(), ServerPredictionKey, GetOwningComponent());
	}
	StopAction(GetOwningActor());
}


void URogueAction_ProjectileAttack::PredictionTimeout_Elapsed()
{
	if (IsRunning() && ServerPredictionKey.IsValid())
	{
		StopAction(GetOwningActor());
	}
}
