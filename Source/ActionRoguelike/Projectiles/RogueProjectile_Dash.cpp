// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectile_Dash.h"
#include "ActionRoguelike.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "GameFramework/GameStateBase.h"
#include "Player/RoguePlayerController.h"
#include "Projectiles/RogueProjectileMovementComponent.h"
#include "Sound/SoundCue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueProjectile_Dash)


ARogueProjectile_Dash::ARogueProjectile_Dash()
{
	MoveComp->InitialSpeed = 6000.f;
}

	
void ARogueProjectile_Dash::BeginPlay()
{
	Super::BeginPlay();
	
	float TimerDelay = DetonateDelay;
	if (HasAuthority())
	{
		// Setup sync time to clients
		TeleportAtTime = GetWorld()->TimeSeconds + DetonateDelay;
	}
	else
	{
		// Should be replicated at this point
		check(!FMath::IsNearlyZero(TeleportAtTime));
		
		TimerDelay = TeleportAtTime - GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
		//UE_LOG(LogGame, Log, TEXT("Adjusted detonation delay: %f"), TimerDelay);
		
		check(TimerDelay > 0.0f);
	}

	GetWorldTimerManager().SetTimer(TimerHandle_DelayedDetonate, this, &ARogueProjectile_Dash::Explode, TimerDelay);
	
	// Check for valid setup during spawn, is used during teleport
	check(GetInstigator());
}


void ARogueProjectile_Dash::Explode_Implementation()
{
	// Clear timer if the Explode was already called through another source like OnActorHit
	GetWorldTimerManager().ClearTimer(TimerHandle_DelayedDetonate);
	
	// Cosmetics
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	NiagaraLoopComp->Deactivate();

	// Make Dormant
	MoveComp->StopMovementImmediately();
	SetActorEnableCollision(false);	

	if (HasAuthority())
	{
		FTimerHandle TimerHandle_DelayedTeleport;
		GetWorldTimerManager().SetTimer(TimerHandle_DelayedTeleport, this, &ARogueProjectile_Dash::TeleportInstigator, TeleportDelay);
	}

	// Skip base implementation as it will destroy actor (we need to stay alive a bit longer to finish the 2nd timer)
	//Super::Explode_Implementation();
}


void ARogueProjectile_Dash::TeleportInstigator()
{
	check(HasAuthority());
	
	APawn* PawnToTeleport = Cast<APawn>(GetInstigator());
	if (IsValid(PawnToTeleport) && URogueGameplayFunctionLibrary::IsAlive(PawnToTeleport) && PawnToTeleport->IsPawnControlled())
	{
		// Keep instigator rotation or it may end up jarring
		bool bSuccess = PawnToTeleport->TeleportTo(GetActorLocation(), PawnToTeleport->GetActorRotation(), false, false);
		if (bSuccess)
		{
			// Play shake on the player we teleported
			APlayerController* PC = PawnToTeleport->GetController<ARoguePlayerController>();
			// may be invalid or nullptr for AI controlled enemies
			if (IsValid(PC))
			{
				PC->ClientStartCameraShake(ImpactShake);
			}
		}	
	}
	
	// Now we're ready to destroy self
	Destroy();
}


void ARogueProjectile_Dash::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ThisClass, TeleportAtTime, COND_InitialOnly);
}