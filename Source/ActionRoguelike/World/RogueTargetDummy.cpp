// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueTargetDummy.h"

#include "SharedGameplayTags.h"
#include "ActionSystem/RogueActionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Core/RogueMonsterData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueTargetDummy)


ARogueTargetDummy::ARogueTargetDummy()
{
	DefaultSceneRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));
	RootComponent = DefaultSceneRootComp;
	
	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	SkelMeshComp->SetRelativeLocation(FVector(0,0,-90.f));
	SkelMeshComp->SetRelativeRotation(FRotator(0, -90, 0));
	SkelMeshComp->SetupAttachment(DefaultSceneRootComp);

	ActionComp = CreateDefaultSubobject<URogueActionComponent>(TEXT("ActionComp"));
	ActionComp->SetDefaultAttributeSet(URogueHealthAttributeSet::StaticClass());
}


void ARogueTargetDummy::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	ActionComp->GetAttributeListenerDelegate(SharedGameplayTags::Attribute_Health).AddUObject(this, &ThisClass::OnHealthChanged);
}

bool ARogueTargetDummy::AddImpulseAtLocationCustom(FVector Impulse, FVector Location, FName BoneName)
{
	FHitReactConfig& Config = ReferenceMonsterData->HitReactions;
	// Play hit Anim
	SkelMeshComp->GetAnimInstance()->PlaySlotAnimationAsDynamicMontage(
		Config.GetAnimFromAngle(this, Impulse.GetSafeNormal()),
		Config.SlotName,
		Config.BlendInTime,
		Config.BlendOutTime);

	return true;	
}


void ARogueTargetDummy::OnHealthChanged(float NewValue, const FAttributeModification& AttributeMod)
{
	// Read by the Overlay Material to flash
	SkelMeshComp->SetCustomPrimitiveDataFloat(HitFlash_CustomPrimitiveIndex, GetWorld()->TimeSeconds);

	// We can skip rendering this at a distance
	/*SkelMeshComp->SetOverlayMaterialMaxDrawDistance(CachedOverlayMaxDistance);

	// After 1.0seconds we should be finished with the hitflash (re-use the handle to reset timer if we get hit again)
	GetWorldTimerManager().SetTimer(OverlayTimerHandle, [this]()
	{
		// Cheap trick to skip rendering this all the time unless we are actively hit flashing
		SkelMeshComp->SetOverlayMaterialMaxDrawDistance(1);
	}, 1.0f, false);*/
}
