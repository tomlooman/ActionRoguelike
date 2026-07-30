// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAICharacter.h"

#include "AIController.h"
#include "EngineUtils.h"
#include "RogueGameTypes.h"
#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionSystemComponent.h"
#include "ActionSystem/RogueAttributeSet.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/RogueGameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AISense_Damage.h"
#include "World/RoguePatrolPoint.h"


ARogueAICharacter::ARogueAICharacter()
{
	ActionSystemComponent = CreateDefaultSubobject<URogueActionSystemComponent>(TEXT("ActionSystemComp"));
	ActionSystemComponent->SetDefaultAttributeSet(URogueMonsterAttributeSet::StaticClass());
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ARogueAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	GetMesh()->SetOverlayMaterialMaxDrawDistance(1);
	
	ActionSystemComponent->GameplayTagUpdated.AddDynamic(this, &ThisClass::OnGameplayTagUpdated);
	ActionSystemComponent->GetAttributeListener(SharedGameplayTags::Attribute_Health).AddUObject(this, &ThisClass::OnHealthChanged);
}

void ARogueAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
	URogueGameInstance* GI = GetGameInstance<URogueGameInstance>();
	check(GI);
	GI->AliveMonsters.Add(this);
}

void ARogueAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	URogueGameInstance* GI = GetGameInstance<URogueGameInstance>();
	GI->AliveMonsters.RemoveSingleSwap(this, EAllowShrinking::No);
}


void ARogueAICharacter::OnGameplayTagUpdated(FGameplayTag UpdatedTag, int32 NewCount)
{
	// Block/Allow movement
	if (UpdatedTag.MatchesTag(SharedGameplayTags::StatusEffect_Stunned))
	{
		const bool bWasAdded = NewCount > 0;

		EMovementMode NewMoveMode = bWasAdded ? MOVE_None : MOVE_Walking;
		GetCharacterMovement()->SetMovementMode(NewMoveMode);
		
		// Pause ALL logic for Monster
		AAIController* AIC = Cast<AAIController>(GetController());
		// Could be nullptr for dead monsters, in that case we shouldn't reach here anymore
		check(AIC);
		UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AIC->GetBrainComponent());
		check(BTComp);
	
		// Alternatively you could restart the logic to skip any 'waits' it might be in and get out of the stunned
		// picking a new thing to do right away (BTComp->StartLogic() BTComp->StopLogic())
		if (bWasAdded)
		{
			BTComp->PauseLogic("StunApplied");
		}
		else
		{
			BTComp->ResumeLogic("StunRemoved");
		}
		
		// Animation
		if (bWasAdded)
		{
			PlayAnimMontage(StunnedAnimation);
		}
	}
}

FGenericTeamId ARogueAICharacter::GetGenericTeamId() const
{
	if (AAIController* AIC = GetController<AAIController>())
	{
		return AIC->GetGenericTeamId();
	}
	
	return FGenericTeamId::NoTeam;
}

void ARogueAICharacter::OnHealthChanged(FGameplayTag AttributeTag, float NewHealth, float OldHealth)
{
	if (NewHealth <= KINDA_SMALL_NUMBER && !bIsDead)
	{
		HandleKilled();
	}
}

void ARogueAICharacter::HandleKilled()
{
	bIsDead = true;
	
	URogueGameInstance* GI = GetGameInstance<URogueGameInstance>();
	GI->AliveMonsters.RemoveSingleSwap(this, EAllowShrinking::No);
	
	AAIController* AIC = GetController<AAIController>();
	AIC->GetBrainComponent()->StopLogic("Killed");
	
	GetMesh()->bPauseAnims = true;
	
	GetMesh()->SetCollisionProfileName("Ragdoll");
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Drop Loot!
	GetActionSystemComponent()->StartAction(SharedGameplayTags::Action_DropLoot);
	
	SetLifeSpan(10.f);
}

float ARogueAICharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                                    class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	
	const ETeamAttitude::Type Attitude = GetTeamAttitudeTowards(*EventInstigator);
	
	if (IsValid(EventInstigator) && Attitude != ETeamAttitude::Friendly)
	{
		UAISense_Damage::ReportDamageEvent(this, this, EventInstigator->GetPawn(), FMath::Abs(ActualDamage), 
			EventInstigator->GetPawn()->GetActorLocation(), GetActorLocation());
	}
	
	ActionSystemComponent->ApplyAttributeChange(SharedGameplayTags::Attribute_Health, -ActualDamage, Base);

	GetMesh()->SetOverlayMaterialMaxDrawDistance(0);
	
	//GetMesh()->SetScalarParameterValueOnMaterials("TimeToHit", GetWorld()->TimeSeconds);
	GetMesh()->SetCustomPrimitiveDataFloat(0, GetWorld()->TimeSeconds);

	GetWorldTimerManager().SetTimer(OverlayTimerHandle, [this]()
	{
		GetMesh()->SetOverlayMaterialMaxDrawDistance(1);
	}, 1.0f, false);

	return ActualDamage;
}
