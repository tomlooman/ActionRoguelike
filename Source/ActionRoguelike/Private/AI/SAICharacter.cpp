// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAICharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "SAttributeComponent.h"
#include "BrainComponent.h"
#include "SWorldUserWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SActionComponent.h"
#include "SSignificanceManagerSettings.h"
#include "Kismet/GameplayStatics.h"


ASAICharacter::ASAICharacter()
{
	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComp");

	AttributeComp = CreateDefaultSubobject<USAttributeComponent>("AttributeComp");

	ActionComp = CreateDefaultSubobject<USActionComponent>("ActionComp");

	// Ensures we receive a controlled when spawned in the level by our gamemode
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Enabled on mesh to react to incoming projectiles
	GetMesh()->SetGenerateOverlapEvents(true);

	TimeToHitParamName = "TimeToHit";
	TargetActorKey = "TargetActor";

	// Optimization
	SignificanceTag = "AICharacter";
	CurrentSignifance = 1.0f;
}


void ASAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (USignificanceManager* SignificanceManager = FSignificanceManagerModule::Get(GetWorld()))
	{
		const USSignificanceManagerSettings* Settings = GetDefault<USSignificanceManagerSettings>();
		SignificanceData = Settings->GetConfig();
		// Dont bother registering if no config is setup in project
		if (ensure(SignificanceData))
		{
			auto SignificanceFunc = [&](USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& Viewpoint) -> float
			{
				return CalcSignificance(ObjectInfo, Viewpoint);
			};

			auto PostSignificanceFunc = [&](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal)
			{
				PostSignificanceUpdate(ObjectInfo, OldSignificance, Significance, bFinal);
			};

			// Register
			SignificanceManager->RegisterObject(this, SignificanceTag, SignificanceFunc, USignificanceManager::EPostSignificanceType::Sequential, PostSignificanceFunc);
		}
	}
}


void ASAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (USignificanceManager* SignificanceManager = FSignificanceManagerModule::Get(GetWorld()))
	{
		SignificanceManager->UnregisterObject(this);
	}
}


void ASAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	PawnSensingComp->OnSeePawn.AddDynamic(this, &ASAICharacter::OnPawnSeen);
	AttributeComp->OnHealthChanged.AddDynamic(this, &ASAICharacter::OnHealthChanged);
}


void ASAICharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f)
	{
		if (InstigatorActor != this)
		{
			SetTargetActor(InstigatorActor);
		}

		if (ActiveHealthBar == nullptr)
		{
			ActiveHealthBar = CreateWidget<USWorldUserWidget>(GetWorld(), HealthBarWidgetClass);
			if (ActiveHealthBar)
			{
				ActiveHealthBar->AttachedActor = this;
				ActiveHealthBar->AddToViewport();
			}
		}

		GetMesh()->SetScalarParameterValueOnMaterials(TimeToHitParamName, GetWorld()->TimeSeconds);

		// Died
		if (NewHealth <= 0.0f)
		{
			// stop BT
			AAIController* AIC = Cast<AAIController>(GetController());
			if (AIC)
			{
				AIC->GetBrainComponent()->StopLogic("Killed");
			}

			// ragdoll
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName("Ragdoll");

			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCharacterMovement()->DisableMovement();

			// set lifespan
			SetLifeSpan(10.0f);
		}
	}
}


void ASAICharacter::SetTargetActor(AActor* NewTarget)
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC)
	{
		AIC->GetBlackboardComponent()->SetValueAsObject(TargetActorKey, NewTarget);
	}
}


AActor* ASAICharacter::GetTargetActor() const
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC)
	{
		return Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject(TargetActorKey));
	}

	return nullptr;
}


void ASAICharacter::OnPawnSeen(APawn* Pawn)
{
	// Ignore if target already set
	if (GetTargetActor() != Pawn)
	{
		SetTargetActor(Pawn);

		MulticastPawnSeen();
	}
	//DrawDebugString(GetWorld(), GetActorLocation(), "PLAYER SPOTTED", nullptr, FColor::White, 0.5f, true);
}


void ASAICharacter::MulticastPawnSeen_Implementation()
{
	USWorldUserWidget* NewWidget = CreateWidget<USWorldUserWidget>(GetWorld(), SpottedWidgetClass);
	if (NewWidget)
	{
		NewWidget->AttachedActor = this;
		// Index of 10 (or anything higher than default of 0) places this on top of any other widget.
		// May end up behind the minion health bar otherwise.
		NewWidget->AddToViewport(10);
	}
}


float ASAICharacter::CalcSignificance(USignificanceManager::FManagedObjectInfo* ObjectInfo,	const FTransform& Viewpoint) const
{
	const float DistanceSqrd = (GetActorLocation() - Viewpoint.GetLocation()).SizeSquared();

	// Allow override to force specific significance on all, good for perf comparison or debugging.
	//if (SignificanceConfig->ForceSignificance >= 0)
	{
		//return SignificanceConfig->ForceSignificance;
	}

	// Your objects may be hidden, setting them to insignificant will help reduce cost without setting a
	// bunch of settings elsewhere to 'disable' temporarily. (In case of AI you'll want to disable/pause Behavior Tree to properly pause it
	if (IsHidden())
	{
		// @todo: experimental, disable entirely while negative significance
		return -1.0f;
	}
		
	return SignificanceData->GetSignificanceByDistance(SignificanceTag, DistanceSqrd);
}


void ASAICharacter::PostSignificanceUpdate(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance,	float Significance, bool bFinal)
{
	// Nothing new to do here...
	if (OldSignificance == Significance)
	{
		return;
	}

	CurrentSignifance = Significance;

	float DesiredTickInterval;
	EVisibilityBasedAnimTickOption SkelMeshTickOption;
	bool bEnableURO = false;

	// @todo: the KEYS from the curve should match this if/else block. meaning 3 curve keys = 3 if/else
	// From highest to lowest significance (straight copy from left-to-right from Curve in config DataAsset)
	// if (Significance == SignificanceData->GetSignificanceByThreshold(SignificanceTag, 0) || Significance < 0.0f) // Catch -1.f in case significance failed anywhere
	// {
	// 	DesiredTickInterval = 0.0f;
	// 	SkelMeshTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	// }
	// else if (Significance == SignificanceData->GetSignificanceByThreshold(SignificanceTag,1))
	// {
	// 	DesiredTickInterval = 0.1f;
	// 	SkelMeshTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	// }
	// else
	// {
	// 	DesiredTickInterval = 0.25f;
	// 	SkelMeshTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	// }

	bool bPauseBT = false;
	

	if (Significance == 0.0f)
	{
		DesiredTickInterval = 0.2f;
		SkelMeshTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

		// Update Rate Optimization
		bEnableURO = true;
	}
	else if (Significance < 1.0f)
	{
		DesiredTickInterval = 0.1f;
		
		SkelMeshTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	}
	else if (Significance >= 1.0f) // Anything at or above 1.0 we consider 'significant' in this example.
	{
		DesiredTickInterval = 0.0f;
		SkelMeshTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
	else
	{
		// pause/freeze entirely
		bPauseBT = true;
		DesiredTickInterval = 1.0f; // @todo: maybe 'pause' ticking
		SkelMeshTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		
		// Invalid significance (eg. -1.0) through some error or invalid setup elsewhere
		// @todo: warn log
		//return;
	}

	// Allow BT to be entirely paused while insignificant (<0.0f)
	// @todo: this seems entirely broken?
	if (AAIController* MyAIC = Cast<AAIController>(GetController()))
	{
		UBrainComponent* BrainComp = MyAIC->GetBrainComponent();
		if (bPauseBT && !BrainComp->IsPaused())
		{
			MyAIC->GetBrainComponent()->PauseLogic("Became Insignificant");
		}
		else if (!bPauseBT && BrainComp->IsPaused())
		{
			BrainComp->ResumeLogic("Became Significant");
		}
	}
	
	SetActorTickInterval(DesiredTickInterval);
	// You may want to reduce movement tick rate, this will make it visibly choppy so use only for super irrelevant AI
	//GetCharacterMovement()->SetComponentTickInterval(DesiredTickInterval);
	
	GetMesh()->VisibilityBasedAnimTickOption = SkelMeshTickOption;
	// Allow engine to skip anim update frames when 'far away' (based on screen size)
	GetMesh()->bEnableUpdateRateOptimizations = bEnableURO;

	// You can use this to visualize "URO" (See comment on variable for more details)
	//GetMesh()->bDisplayDebugUpdateRateOptimizations = true;
}


bool ASAICharacter::IsSignificant(float RequiredSignificance) const
{
	return CurrentSignifance >= RequiredSignificance;
}
