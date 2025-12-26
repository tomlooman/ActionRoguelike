// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueTreasureChest.h"

#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Animation/RogueCurveAnimSubsystem.h"
#include "Components/AudioComponent.h"
#include "Core/RogueDeferredTaskSystem.h"

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

#if USE_DEFERRED_TASKS
	// Experimenting with the deferred tasks, no need to actually do this for the chest
	URogueDeferredTaskSystem::AddLambda(this,[&]()
		{
			ConditionalOpenChest();
		});

	// Some random numbers to "process"
	IntTestArray = { 4, 5, 7, 574, 4737, 4837, 3726 };

	URogueDeferredTaskSystem::AddLambda(this,[&]()
		{
			UpdateTestArray(4, 8);
		});
#else
	ConditionalOpenChest();
#endif
}

void ARogueTreasureChest::UpdateTestArray(int32 StartIndex, int32 MaxCount)
{
#if USE_DEFERRED_TASKS
	// Process partial array, so it can be split across multiple frames easily
	int32 CurrentCount = 0;
	for (int32 i = StartIndex; i < IntTestArray.Num(); i++)
	{
		IntTestArray[i] = IntTestArray[i] + 1;
		CurrentCount++;
		if (CurrentCount >= MaxCount)
		{
			break;
		}
	}
#endif
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

		// Several ways to trigger and manage these animations (curve-based automatic ticking, manual ticking, and easing functions)
		
		AnimSubsystem->PlayCurveAnim(LidAnimCurve, 1.f, [&](float CurrValue)
		{
			LidMesh->SetRelativeRotation(FRotator(CurrValue, 0, 0));
		});
		
		// manually handled variation to tick yourself
		/*CurveAnimInst = new FActiveCurveAnim(LidAnimCurve, [&](float CurrValue)
		{
			LidMesh->SetRelativeRotation(FRotator(CurrValue, 0, 0));
		}, 1.0f);*/

		/*
		AnimSubsystem->PlayEasingFunc(EEasingFunc::EaseInOut, 2.0f, 2.0f, [&](float CurrValue)
		{
			LidMesh->SetRelativeRotation(FRotator(CurrValue * 100.f, 0, 0));
		});*/

		OpenChestEffect->Activate(true);

		OpenChestSound->Play();

	}
}

/*
void ARogueTreasureChest::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Example of manually ticking the animation, may be useful if you need the control and/or manually batch the anims
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
