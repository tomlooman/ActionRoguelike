// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RogueAICharacter.h"

#include "ActionRoguelike.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "BrainComponent.h"
#include "NiagaraComponent.h"
#include "SharedGameplayTags.h"
#include "SignificanceManager.h"
#include "SkeletalMeshComponentBudgeted.h"
#include "UI/RogueWorldUserWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActionSystem/RogueActionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CanvasPanel.h"
#include "Perception/AISense_Damage.h"
#include "IAnimationBudgetAllocator.h"
#include "NavigationSystem.h"
#include "AnimationBudgetAllocator/Private/AnimationBudgetAllocatorModule.h"
#include "Core/RogueMessagingSubsystem.h"
#include "Pickups/RoguePickupSubsystem.h"
#include "Subsystems/RogueMonsterCorpseSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueAICharacter)


ARogueAICharacter::ARogueAICharacter(const FObjectInitializer& ObjectInitializer)
	// Override the SkelMesh with the Anim Budget variant for balancing anim cost across all AI bots
	:Super(ObjectInitializer.SetDefaultSubobjectClass<USkeletalMeshComponentBudgeted>(ACharacter::MeshComponentName))
{
	ActionComp = CreateDefaultSubobject<URogueActionComponent>(TEXT("ActionComp"));
	ActionComp->SetDefaultAttributeSet(URogueMonsterAttributeSet::StaticClass());

	AttackSoundComp = CreateDefaultSubobject<UAudioComponent>(TEXT("AttackAudioComp"));
	AttackSoundComp->SetupAttachment(RootComponent);
	AttackSoundComp->bAutoManageAttachment = true;
	AttackSoundComp->SetAutoActivate(false);

	AttackParticleComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttackParticleComp"));
	AttackParticleComp->SetupAttachment(GetMesh(), AttackFX_Socket);
	AttackParticleComp->bAutoManageAttachment = true;
	AttackParticleComp->SetAutoActivate(false);

	// Ensures we receive a controlled when spawned in the level by our gamemode
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Enabled on mesh to react to incoming projectiles
	USkeletalMeshComponent* SkelMesh = GetMesh();
	SkelMesh->SetGenerateOverlapEvents(true);
	// Skip performing overlap queries on the Physics Asset after animation (17 queries in case of our MinionRangedBP)
	SkelMesh->bUpdateOverlapsOnAnimationFinalize = false;
	// Skip bones when not visible, may miss anim notifies etc. if animation is skipped so these options must be tested per use case
	SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickMontagesWhenNotRendered;
}

void ARogueAICharacter::BeginPlay()
{
	Super::BeginPlay();

	// Only needs to enable the module once, placing in beginplay for convenience
	// They didn't expose the blueprint library, so we instead call directly into the module
	FAnimationBudgetAllocatorModule& AnimationBudgetAllocatorModule = FModuleManager::LoadModuleChecked<FAnimationBudgetAllocatorModule>("AnimationBudgetAllocator");
	if(IAnimationBudgetAllocator* AnimationBudgetAllocator = AnimationBudgetAllocatorModule.GetBudgetAllocatorForWorld(GetWorld()))
	{
		AnimationBudgetAllocator->SetEnabled(true);
	}

	// Significance Manager
	{
		USignificanceManager* SigMan = USignificanceManager::Get(GetWorld());
		check(SigMan);

		// This function will run async from the GameThread, so make sure it's threadsafe
		auto SignificanceFunc = [](USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& Viewpoint) -> float
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(CalculateSignificance)

			// Either use the Actor as done below, or scope this more tightly to a specific component, this may benefit from improved cpu caching
			/*AActor* MyActor = CastChecked<AActor>(ObjectInfo->GetObject());
			check(IsValid(MyActor));
			
			float DistanceSqrt = (MyActor->GetActorLocation() - Viewpoint.GetLocation()).SizeSquared();
			
			if (MyActor->WasRecentlyRendered())
			{
				// while using negative sig values, use a <1.0 multiplier
				DistanceSqrt *= 0.5f;
			}*/

			USkeletalMeshComponent* SkelMeshComp = CastChecked<USkeletalMeshComponent>(ObjectInfo->GetObject());
			check(IsValid(SkelMeshComp));
			
			float DistanceSqrt = (SkelMeshComp->GetComponentLocation() - Viewpoint.GetLocation()).SizeSquared();
			
			if (SkelMeshComp->WasRecentlyRendered())
			{
				// while using negative sig values, use a <1.0 multiplier
				DistanceSqrt *= 0.5f;
			}

			// Note: AI could further define significance, for example, while in combat or having the player as a known target we could increase its significance

			// Negative distance to easily have larger distance mean lower significance
			return -DistanceSqrt;
		};

		// Register with post significance function to easily tie-in with the animation budgeter
		// We could also choose to let the budgeter calculate the significance itself instead
		auto PostSignificanceFunc = [&](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal)
		{
			USkeletalMeshComponentBudgeted* BudgetMesh = Cast<USkeletalMeshComponentBudgeted>(GetMesh());
			BudgetMesh->SetComponentSignificance(Significance);
		};

		// Additional flag in the budgetter to allow us to 'toggle' and turn off certain animation features custom to the game, this could mean detaching components on our skeletal mesh
		// it's entirely game dependent on what we could throttle here
		USkeletalMeshComponentBudgeted* BudgetMesh = Cast<USkeletalMeshComponentBudgeted>(GetMesh());
		BudgetMesh->OnReduceWork().BindUObject(this, &ARogueAICharacter::OnReduceAnimationWork);

		// If not using Significance Manager already, we could let it calculate automatically instead
		//BudgetMesh->SetAutoCalculateSignificance(true);

		// Instead of passing the entire Actor, we can pass the minimal data, such as the RootComponent, or SkeletalMeshComponent
		// This should allow us to be more cache efficient (from simple testing this does run slightly faster than using the Actor)
		SigMan->RegisterObject(GetMesh(), SignificanceTag, SignificanceFunc, USignificanceManager::EPostSignificanceType::Concurrent, PostSignificanceFunc);
	}
}


void ARogueAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Remove from SigMan
	{
		USignificanceManager* SigMan = USignificanceManager::Get(GetWorld());
		check(SigMan);
		// Make sure we unregister the same object as during Registration, in our case that's the SkeletalMeshComponent instead of the Actor
		SigMan->UnregisterObject(GetMesh());
	}

	Super::EndPlay(EndPlayReason);
}


void ARogueAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	// Cheap trick to disable until we need it in the health event
	CachedOverlayMaxDistance = GetMesh()->OverlayMaterialMaxDrawDistance;
	GetMesh()->SetOverlayMaterialMaxDrawDistance(1);

	FAttributeChangedSignature& Delegate = ActionComp->GetAttributeListenerDelegate(SharedGameplayTags::Attribute_Health);
	Delegate.AddUObject(this, &ThisClass::OnHealthAttributeChanged);
}


void ARogueAICharacter::OnHealthAttributeChanged(float NewValue, const FAttributeModification& AttributeModification)
{
	float Delta = AttributeModification.Magnitude;
	AActor* InstigatorActor = AttributeModification.Instigator.Get();
	
	if (Delta < 0.0f)
	{
		// Create once, and skip on instant kill
		if (ActiveHealthBar == nullptr && NewValue > 0.0)
		{
			ActiveHealthBar = CreateWidget<URogueWorldUserWidget>(GetWorld(), HealthBarWidgetClass);
			if (ActiveHealthBar)
			{
				ActiveHealthBar->AttachedActor = this;
				URogueWorldUserWidget::AddToRootCanvasPanel(ActiveHealthBar);
			}
		}

		// Read by the Overlay Material to flash
		GetMesh()->SetCustomPrimitiveDataFloat(HitFlash_CustomPrimitiveIndex, GetWorld()->TimeSeconds);

		// We can skip rendering this at a distance
		GetMesh()->SetOverlayMaterialMaxDrawDistance(CachedOverlayMaxDistance);

		// After 1.0seconds we should be finished with the hitflash (re-use the handle to reset timer if we get hit again)
		GetWorldTimerManager().SetTimer(OverlayTimerHandle, [this]()
		{
			// Cheap trick to skip rendering this all the time unless we are actively hit flashing
			GetMesh()->SetOverlayMaterialMaxDrawDistance(1);
		}, 1.0f, false);

		// Died
		if (NewValue <= 0.0f)
		{
			// stop BT
			if (HasAuthority())
			{
				AAIController* AIC = GetController<AAIController>();
				AIC->GetBrainComponent()->StopLogic("Killed");

				// Clears active actions, and (de)buffs.
				ActionComp->StopAllActions();

#if USE_DOD_CREDIT_PICKUPS
				// spawn credit loot, spawn a ton of them for stress testing
				URoguePickupSubsystem* PickupSubsystem = GetWorld()->GetSubsystem<URoguePickupSubsystem>();
				FVector ActorLoc = GetActorLocation();
				const FVector Offset = FVector(0,0,30);

				const int32 SpawnCount = 100;
				for (int i = 0; i < SpawnCount; ++i)
				{
					FNavLocation OutNavLoc;
					UNavigationSystemV1::GetNavigationSystem(this)->GetRandomPointInNavigableRadius(ActorLoc, 1024, OutNavLoc);

					PickupSubsystem->AddCreditsPickup(OutNavLoc.Location + Offset, 10);
				}
#endif

#if USE_TAGMESSAGING_SYSTEM
				FPayLoadTestMessage MsgPayload;
				MsgPayload.Credits = 25;

				URogueMessagingSubsystem* Msg = UGameInstance::GetSubsystem<URogueMessagingSubsystem>(GetGameInstance());
				Msg->BroadcastTagNative(SharedGameplayTags::Message_MonsterKilled, MsgPayload);
#endif
				
			}

			// ragdoll
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName(Collision::Ragdoll_ProfileName);

			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCharacterMovement()->DisableMovement();

			//SetLifeSpan(10.0f);
			URogueMonsterCorpseSubsystem* CorpseSystem = GetWorld()->GetSubsystem<URogueMonsterCorpseSubsystem>();
			CorpseSystem->AddCorpse(this);
			return;
		}

		// Damaged, but not dead yet

		// AI logic only runs on server
		if (HasAuthority())
		{
			// Skip reporting damage event for "Friendly" units. (We could also catch this earlier and prevent friendly-fire between AI units)
			ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*InstigatorActor);
			if (Attitude != ETeamAttitude::Friendly)
			{
				UAISense_Damage::ReportDamageEvent(this, this, InstigatorActor, FMath::Abs(Delta),
					InstigatorActor->GetActorLocation(), GetActorLocation());
			}
		}
	}
}


AActor* ARogueAICharacter::GetTargetActor() const
{
	AAIController* AIC = GetController<AAIController>();
	// Not guaranteed to be possessed (as we may use this in the AnimBP)
	if (AIC)
	{
		return Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject(NAME_TargetActor));
	}
	return nullptr;
}


void ARogueAICharacter::MulticastPlayAttackFX_Implementation()
{
	AttackSoundComp->Play();

	AttackParticleComp->Activate(true);
	
	PlayAnimMontage(AttackMontage);
}


void ARogueAICharacter::SignificanceLODChanged(int32 NewLOD)
{
	UE_LOG(LogGame, Verbose, TEXT("Actor: %s, NewLOD: %i (Bucket)"), *GetName(), NewLOD);

	EMovementMode MoveMode = NewLOD > 0 ? MOVE_NavWalking : MOVE_Walking;
	// GroundMovementMode won't mess with Flying/Falling modes
	GetCharacterMovement()->SetGroundMovementMode(MoveMode);

	// Example with straight 1:1 mapping, will force the min LOD to be lowered even when they are close to the camera
	GetMesh()->OverrideMinLOD(NewLOD);
}


void ARogueAICharacter::OnReduceAnimationWork(class USkeletalMeshComponentBudgeted* InComponent, bool bReduce)
{
	UE_LOG(LogGame, Log, TEXT("OnReduceAnimWork for bot %s, reducing = %s"), *GetName(), (bReduce ? TEXT("true") : TEXT("false")));

	// @todo: Actually throttle some work, for example, detach certain components on the skeletal mesh IF we had any in the first place
}


FGenericTeamId ARogueAICharacter::GetGenericTeamId() const
{
	// Matches the AIController team ID
	return FGenericTeamId(TEAM_ID_BOTS);
}
