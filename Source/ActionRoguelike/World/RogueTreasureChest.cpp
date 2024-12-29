// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueTreasureChest.h"

#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/RogueCurveAnimSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueTreasureChest)


ARogueTreasureChest::ARogueTreasureChest()
{
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	RootComponent = BaseMesh;

	LidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMesh"));
	LidMesh->SetupAttachment(BaseMesh);

	OpenChestEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("OpeningEffectComp"));
	OpenChestEffect->SetupAttachment(RootComponent);
	OpenChestEffect->bAutoActivate = false;
	// only attach while playing the VFX, this skips transform updates when the chests moves around the world
	// while the VFX is not active
	OpenChestEffect->bAutoManageAttachment = true;

	// Directly set bool instead of going through SetReplicates(true) within constructor,
	// Only use SetReplicates() outside constructor
	bReplicates = true;

	//PrimaryActorTick.bCanEverTick = true;
}


void ARogueTreasureChest::Interact_Implementation(APawn* InstigatorPawn)
{
	bLidOpened = true;
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
		// @todo: lidmesh still as replicated relative rotation?
		
		URogueCurveAnimSubsystem* AnimSubsystem = GetWorld()->GetSubsystem<URogueCurveAnimSubsystem>();
		AnimSubsystem->PlayCurveAnim(LidAnimCurve, 1.0f, [&](float CurrValue)
		{
			LidMesh->SetRelativeRotation(FRotator(CurrValue, 0, 0));
		});

		OpenChestEffect->Activate(true);

		// manually handled variation to tick yourself
		/*CurveAnimInst = new FActiveCurveAnim(LidAnimCurve, [&](float CurrValue)
		{
			LidMesh->SetRelativeRotation(FRotator(CurrValue, 0, 0));
		}, 1.0f);*/
	}
}

/*
void ARogueTreasureChest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurveAnimInst && CurveAnimInst->IsValid())
	{
		CurveAnimInst->Tick(DeltaSeconds);
	}
}*/


void ARogueTreasureChest::OnRep_LidOpened()
{
	ConditionalOpenChest();
}


void ARogueTreasureChest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARogueTreasureChest, bLidOpened);
}
