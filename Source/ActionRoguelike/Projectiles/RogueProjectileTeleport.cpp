// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectileTeleport.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"


ARogueProjectileTeleport::ARogueProjectileTeleport()
{
	ProjectileMovementComponent->InitialSpeed = 6000.0f;
}

void ARogueProjectileTeleport::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TeleportHandle, this, &ThisClass::StartDelayedTeleport, DetonateDelay);
}

void ARogueProjectileTeleport::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// Skip the base implementation, we handle our own as we must delay destroying and avoid playing duplicate explosion effects
	//Super::OnActorHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	// Cancel the timer to prevent a second teleportation
	GetWorldTimerManager().ClearTimer(TeleportHandle);

	StartDelayedTeleport();
}


void ARogueProjectileTeleport::StartDelayedTeleport()
{
	PlayExplodeEffects();
	
	// Prevent moving us further ahead while we wait for the teleport to trigger
	ProjectileMovementComponent->StopMovementImmediately();
	// Hide all visuals and prevent any further collision while we wait on the teleport timer
	LoopedNiagaraComponent->Deactivate();
	LoopedAudioComponent->Stop();
	SetActorEnableCollision(false);
	
	// Another delay until we teleport (so player has a chance to see the VFX before we move them)
	GetWorldTimerManager().SetTimer(TeleportHandle, this, &ThisClass::HandleTeleportation, TeleportSecondaryDelay);
}

void ARogueProjectileTeleport::HandleTeleportation()
{
	APawn* ActorToTeleport = GetInstigator();
	// todo: We should check if instigator is still valid/alive once we implement Death
	check(ActorToTeleport);

	// Handles de-penetration on nearby collision geometry
	ActorToTeleport->TeleportTo(GetActorLocation(), ActorToTeleport->GetActorRotation());
	// note: the teleport call might fail if it cannot find any valid location

	// Clear projectile from world, can't do this any sooner as that would prevent the timers from running on a valid Actor
	Destroy();
}
