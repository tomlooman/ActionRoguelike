// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueTreasureChest.h"

#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/RogueCurveAnimSubsystem.h"
#include "Components/AudioComponent.h"
#include "Core/RogueDeferredTaskSystem.h"
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
	bLidOpened = true;
	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, bLidOpened, this);
	
	ConditionalOpenChest();
}


void ARogueTreasureChest::OnActorLoaded_Implementation()
{
	ConditionalOpenChest();
}


void ARogueTreasureChest::ConditionalOpenChest()
{
	if (bLidOpened)
	{
		URogueCurveAnimSubsystem* AnimSubsystem = GetWorld()->GetSubsystem<URogueCurveAnimSubsystem>();
		AnimSubsystem->PlayCurveAnim(LidAnimCurve, 1.f, [&](float CurrValue)
		{
			LidMesh->SetRelativeRotation(FRotator(CurrValue, 0, 0));
		});

		OpenChestEffect->Activate(true);

		OpenChestSound->Play();
	}
}


void ARogueTreasureChest::OnRep_LidOpened()
{
	ConditionalOpenChest();
}


void ARogueTreasureChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Opt-in for Iris Push Model
	FDoRepLifetimeParams PushParams;
	PushParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, bLidOpened, PushParams);
}
