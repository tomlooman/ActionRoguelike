// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectile_Dash.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Player/RoguePlayerController.h"
#include "Projectiles/RogueProjectileMovementComponent.h"
#include "Sound/SoundCue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueProjectile_Dash)




ARogueProjectile_Dash::ARogueProjectile_Dash()
{
	TeleportDelay = 0.2f;
	DetonateDelay = 0.2f;

	MoveComp->InitialSpeed = 6000.f;
}


void ARogueProjectile_Dash::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TimerHandle_DelayedDetonate, this, &ARogueProjectile_Dash::Explode, DetonateDelay);
}


void ARogueProjectile_Dash::Explode_Implementation()
{
	// Clear timer if the Explode was already called through another source like OnActorHit
	GetWorldTimerManager().ClearTimer(TimerHandle_DelayedDetonate);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());

	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());

	NiagaraLoopComp->Deactivate();

	MoveComp->StopMovementImmediately();
	SetActorEnableCollision(false);

	FTimerHandle TimerHandle_DelayedTeleport;
	GetWorldTimerManager().SetTimer(TimerHandle_DelayedTeleport, this, &ARogueProjectile_Dash::TeleportInstigator, TeleportDelay);

	// Skip base implementation as it will destroy actor (we need to stay alive a bit longer to finish the 2nd timer)
	//Super::Explode_Implementation();
}


void ARogueProjectile_Dash::TeleportInstigator()
{
	AActor* ActorToTeleport = GetInstigator();
	// Must be set in spawning code (also good for things like knowing who caused the damage)
	check(ActorToTeleport);

	// Keep instigator rotation or it may end up jarring
	ActorToTeleport->TeleportTo(GetActorLocation(), ActorToTeleport->GetActorRotation(), false, false);

	// Play shake on the player we teleported
	const APawn* InstigatorPawn = CastChecked<APawn>(ActorToTeleport);
	APlayerController* PC = InstigatorPawn->GetController<ARoguePlayerController>();
	// Controller can be nullptr if we died (and detached the pawn) just after launching the dash projectile
	if (PC && PC->IsLocalController())
	{
		PC->PlayerCameraManager->StartCameraShake(ImpactShake);
	}

	// Now we're ready to destroy self
	Destroy();
}
