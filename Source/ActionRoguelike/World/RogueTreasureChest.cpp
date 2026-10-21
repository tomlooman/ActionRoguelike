// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueTreasureChest.h"

#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/RogueCurveAnimSubsystem.h"
#include "Components/AudioComponent.h"
#include "Core/RogueDeferredTaskSystem.h"
#include "GameFramework/GameStateBase.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueTreasureChest)



ARogueTreasureChest::ARogueTreasureChest()
{
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	RootComponent = BaseMesh;
	BaseMesh->SetSimulatePhysics(true);

	LidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMesh"));
	LidMesh->SetupAttachment(BaseMesh);

	OpenChestEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("OpeningEffectComp"));
	OpenChestEffect->SetupAttachment(RootComponent);
	OpenChestEffect->bAutoActivate = false;
	// only attach while playing the VFX, this skips transform updates when the chests moves around the world
	// while the VFX is not active
	OpenChestEffect->bAutoManageAttachment = true;

	// If the chest was non-movable we could just call "playsoundatlocation" and skip creating a component during spawn
	OpenChestSound = CreateDefaultSubobject<UAudioComponent>(TEXT("OpenChestSFX"));
	OpenChestSound->SetupAttachment(RootComponent);
	OpenChestSound->SetAutoActivate(false);
	OpenChestSound->bAutoManageAttachment = true;

	// Directly set bool instead of going through SetReplicates(true) within constructor,
	// Only use SetReplicates() outside constructor
	bReplicates = true;

	//PrimaryActorTick.bCanEverTick = true;
}


void ARogueTreasureChest::Interact_Implementation(AController* InstigatorController)
{
	if (!bLidOpened)
	{
		// Set state, for re-loading the world, animation handled through timestamp
		bLidOpened = true;
		check(HasAuthority());
		// Sets server time
		InteractedServerTime = GetWorld()->TimeSeconds;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, InteractedServerTime, this);
	
		StartOpenChestAnim();
	}
}


void ARogueTreasureChest::OnActorLoaded_Implementation()
{
	if (bLidOpened)
	{
		SetChestFullyOpened();
	}
}


void ARogueTreasureChest::StartOpenChestAnim()
{
	URogueCurveAnimSubsystem* AnimSubsystem = GetWorld()->GetSubsystem<URogueCurveAnimSubsystem>();
	AnimSubsystem->PlayCurveAnim(LidAnimCurve, 1.f, [&](float CurrValue)
	{
		LidMesh->SetRelativeRotation(FRotator(CurrValue, 0, 0));
	});

	OpenChestEffect->Activate(true);

	OpenChestSound->Play();
}


void ARogueTreasureChest::SetChestFullyOpened()
{
	// Set state to already opened, skipping VFX playback
	URogueCurveAnimSubsystem* AnimSubsystem = GetWorld()->GetSubsystem<URogueCurveAnimSubsystem>();
	float FinalPitchRotation = AnimSubsystem->GetAnimationEndValue(LidAnimCurve);
	LidMesh->SetRelativeRotation(FRotator(FinalPitchRotation, 0, 0));
}


void ARogueTreasureChest::OnRep_InteractedTime()
{
	// Assume this is not called until we set a valid time
	check(InteractedServerTime > 0.0f);

	if (AGameStateBase* GS = GetWorld()->GetGameState())
	{
		float CurrentServerTime = GS->GetServerWorldTimeSeconds();
		
		// Could it happen we don't receive an initial valid server time during late join?
		check(CurrentServerTime > 0.0f);
		
		float TimeSinceOpened = CurrentServerTime - InteractedServerTime;
		
		// @fixme: find actual animation time with some buffer
		const float AssumedAnimDuration = 2.0f;
		
		if (TimeSinceOpened > AssumedAnimDuration)
		{
			SetChestFullyOpened();
		}
		else
		{
			StartOpenChestAnim();
		}
	}
	else
	{
		// no comparison available, maybe GameState hasn't arrived yet.
		SetChestFullyOpened();
	}
}


void ARogueTreasureChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Opt-in for Iris Push Model
	FDoRepLifetimeParams PushParams;
	PushParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, InteractedServerTime, PushParams);
}
