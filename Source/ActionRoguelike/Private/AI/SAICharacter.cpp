// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/SAICharacter.h"

#include "ActionRoguelike.h"
#include "AIController.h"
#include "DrawDebugHelpers.h"
#include "SAttributeComponent.h"
#include "BrainComponent.h"
#include "SWorldUserWidget.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SActionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/SSignificanceComponent.h"
#include "Perception/AISense_Damage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SAICharacter)


ASAICharacter::ASAICharacter()
{
	AttributeComp = CreateDefaultSubobject<USAttributeComponent>(TEXT("AttributeComp"));
	ActionComp = CreateDefaultSubobject<USActionComponent>(TEXT("ActionComp"));

	// Make sure to configure the distance values in Blueprint
	SigManComp = CreateDefaultSubobject<USSignificanceComponent>(TEXT("SigManComp"));

	// Ensures we receive a controlled when spawned in the level by our gamemode
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Enabled on mesh to react to incoming projectiles
	USkeletalMeshComponent* SkelMesh = GetMesh();
	SkelMesh->SetGenerateOverlapEvents(true);
	// Skip performing overlap queries on the Physics Asset after animation (17 queries in case of our MinionRangedBP)
	SkelMesh->bUpdateOverlapsOnAnimationFinalize = false;

	// Skip bones when not visible, may miss anim notifies etc. if animation is skipped so these options must be tested per use case
	SkelMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	//TimeToHitParamName = "TimeToHit";
	HitFlash_CustomPrimitiveIndex = 0;
}


void ASAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AttributeComp->OnHealthChanged.AddDynamic(this, &ASAICharacter::OnHealthChanged);
	SigManComp->OnSignificanceChanged.AddDynamic(this, &ASAICharacter::OnSignificanceChanged);
}


void ASAICharacter::OnHealthChanged(AActor* InstigatorActor, USAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f)
	{
		// Create once, and skip on instant kill
		if (ActiveHealthBar == nullptr && NewHealth > 0.0)
		{
			ActiveHealthBar = CreateWidget<USWorldUserWidget>(GetWorld(), HealthBarWidgetClass);
			if (ActiveHealthBar)
			{
				ActiveHealthBar->AttachedActor = this;
				USWorldUserWidget::AddToRootCanvasPanel(ActiveHealthBar);
			}
		}

		// Replaces the above "old" method of requiring unique material instances for every mesh element on the player 
		GetMesh()->SetCustomPrimitiveDataFloat(HitFlash_CustomPrimitiveIndex, GetWorld()->TimeSeconds);

		// Died
		if (NewHealth <= 0.0f)
		{
			// stop BT
			AAIController* AIC = GetController<AAIController>();
			AIC->GetBrainComponent()->StopLogic("Killed");

			// ragdoll
			GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetMesh()->SetCollisionProfileName(Collision::Ragdoll_ProfileName);

			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCharacterMovement()->DisableMovement();

			// set lifespan
			SetLifeSpan(10.0f);
		}
		else
		{
			UAISense_Damage::ReportDamageEvent(this, this, InstigatorActor, FMath::Abs(Delta),
				InstigatorActor->GetActorLocation(), GetActorLocation());
		}
	}
}


void ASAICharacter::MulticastPawnSeen_Implementation()
{
	USWorldUserWidget* NewWidget = CreateWidget<USWorldUserWidget>(GetWorld(), SpottedWidgetClass);
	// Can be nullptr if we didnt specify a class to use in Blueprint
	if (NewWidget)
	{
		NewWidget->AttachedActor = this;
		USWorldUserWidget::AddToRootCanvasPanel(NewWidget);
	}
}


AActor* ASAICharacter::GetTargetActor() const
{
	AAIController* AIC = GetController<AAIController>();
	return Cast<AActor>(AIC->GetBlackboardComponent()->GetValueAsObject("TargetActor"));
}

void ASAICharacter::OnSignificanceChanged(ESignificanceValue Significance)
{
	// @todo: this may not work perfectly with falling and similar movement modes. (We don't support this on the AI character anyway)
	// NavMesh based walking instead of using world geo

	// @todo: this can crash in Chaos due to not being on the GameThread?
	if (Significance <= ESignificanceValue::Medium)
	{
		//GetCharacterMovement()->SetMovementMode(MOVE_NavWalking);
	}
	else
	{
		//GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	
	// Set as 'dormant' if actor is hidden, otherwise we continue ticking the entire character
	const bool bHiddenSignificance = Significance == ESignificanceValue::Hidden;
	SetActorTickEnabled(!bHiddenSignificance);
	GetCharacterMovement()->SetComponentTickEnabled(!bHiddenSignificance);


	EVisibilityBasedAnimTickOption AnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	switch (Significance)
	{
		// Example, force to always tick pose when really nearby. might need the pose even while offscreen
		case ESignificanceValue::Highest:
			AnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
			break;
		case ESignificanceValue::Medium:
			AnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
			break;
		case ESignificanceValue::Lowest:
		case ESignificanceValue::Hidden:
		case ESignificanceValue::Invalid:
			AnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	}
	
	GetMesh()->VisibilityBasedAnimTickOption = AnimTickOption;
}