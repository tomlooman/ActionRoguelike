// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueProjectile.h"
#include "Components/SphereComponent.h"
#include "Projectiles/RogueProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "ProfilingDebugging/CountersTrace.h"
#include "Performance/RogueActorPoolingSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueProjectile)

TRACE_DECLARE_INT_COUNTER(COUNTER_GAME_ActiveProjectiles, TEXT("Game/ActiveProjectiles"));

ARogueProjectile::ARogueProjectile()
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
	
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetCollisionProfileName("Projectile");
	// Dont bother telling the nav system whenever we move
	SphereComp->SetCanEverAffectNavigation(false);
	RootComponent = SphereComp;

	NiagaraLoopComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("EffectComp"));
	//NiagaraLoopComp->PoolingMethod = ENCPoolMethod::AutoRelease;
	NiagaraLoopComp->SetupAttachment(RootComponent);

	AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComp"));
	AudioComp->SetupAttachment(RootComponent);

	// Custom Projectile Component (for tick management & better homing)
	MoveComp = CreateDefaultSubobject<URogueProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	MoveComp->bRotationFollowsVelocity = true;
	MoveComp->bInitialVelocityInLocalSpace = true;
	MoveComp->ProjectileGravityScale = 0.0f;
	MoveComp->InitialSpeed = 8000;

	// Directly set bool instead of going through SetReplicates(true) within constructor,
	// Only use SetReplicates() outside constructor
	bReplicates = true;
}


void ARogueProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// More consistent to bind here compared to Constructor which may fail to bind if Blueprint was created before adding this binding (or when using hotreload)
	// PostInitializeComponent is the preferred way of binding any events.
	SphereComp->OnComponentHit.AddDynamic(this, &ARogueProjectile::OnActorHit);
}

void ARogueProjectile::BeginPlay()
{
	Super::BeginPlay();

	// Can use to fine-tune the pre allocated actor pool by checking how many projectiles are alive during gameplay
	TRACE_COUNTER_INCREMENT(COUNTER_GAME_ActiveProjectiles);

	// Should be false so that we manually manage the lifetime, better for priming actors
	check(!AudioComp->bAutoActivate);
	check(!NiagaraLoopComp->bAutoActivate);
}

void ARogueProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	TRACE_COUNTER_DECREMENT(COUNTER_GAME_ActiveProjectiles);
}

void ARogueProjectile::PostNetInit()
{
	Super::PostNetInit();

	// Client just spawned this poolable Actor, now either park it or just keep it active...
	// @todo-fixme: this works on primed actors, but later spawned actors will also get parked here...
	PoolEndPlay_Implementation();
}

void ARogueProjectile::PostNetReceive()
{
	Super::PostNetReceive();

	//if (NiagaraLoopComp->IsPaused())
	{
		// instead use a replicated variable that handles the pooling state
		check(false);
		// @todo: only run once, when woken up through replication/dormancy
		PoolBeginPlay_Implementation();
	}
}


void ARogueProjectile::PoolBeginPlay_Implementation()
{
	if (HasAuthority())
	{
		MoveComp->Reset();
	}
	
	StartVFX();
	
	// Unpausing is significantly faster than re-creating renderstates due to Deactivate()
	// Does keep its state around which is OK for our loopable VFX that will mostly be active/in-use
	NiagaraLoopComp->SetPaused(false);
	// Reset to fix ribbon positions
	//NiagaraLoopComp->ResetSystem();
	AudioComp->SetPaused(false);
}


void ARogueProjectile::PoolEndPlay_Implementation()
{
	//NiagaraLoopComp->Deactivate();
	//AudioComp->Stop();
	NiagaraLoopComp->SetPaused(true);
	AudioComp->SetPaused(true);
}


void ARogueProjectile::StartVFX()
{
	// May both be inactive on first run, coming from a primed pool
	if (!NiagaraLoopComp->IsActive())
	{
		NiagaraLoopComp->Activate();
	}
	if (!AudioComp->IsPlaying())
	{
		AudioComp->Play();
	}
}

void ARogueProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	DrawDebugBox(GetWorld(), GetActorLocation(), FVector(25.0f), FColor::Red, false, 0.0f, SDPG_Foreground);
}


void ARogueProjectile::OnRep_OnPooledStateChanged()
{
	// Client side updates back into the pooling system
	URogueActorPoolingSubsystem* PoolingSubsystem = GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
	
	if (bIsUsedInPool)
	{
		PoolingSubsystem->AcquireFromPool_Internal()
	}
	else
	{
		PoolingSubsystem->ReleaseToPool_Internal(this);
	}
}


void ARogueProjectile::LifeSpanExpired()
{
	// Skip destroy and instead release to pool
	URogueActorPoolingSubsystem::ReleaseToPool(this);
}


void ARogueProjectile::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Explode();
}


// _Implementation from it being marked as BlueprintNativeEvent
void ARogueProjectile::Explode_Implementation()
{
	// Auto-managed particle pooling
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation(), FVector(1), true, true, ENCPoolMethod::AutoRelease);

	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());

	UGameplayStatics::PlayWorldCameraShake(this, ImpactShake, GetActorLocation(), ImpactShakeInnerRadius, ImpactShakeOuterRadius);

	//Destroy();
	// Release back to pool instead of destroying
	URogueActorPoolingSubsystem* PoolingSubsystem = GetWorld()->GetSubsystem<URogueActorPoolingSubsystem>();
	PoolingSubsystem->ReleaseToPool(this);
}

float ARogueProjectile::GetDefaultSpeed() const
{
	return MoveComp->InitialSpeed;
}

float ARogueProjectile::GetGravityScale() const
{
	return MoveComp->ProjectileGravityScale;
}


