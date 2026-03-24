// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RogueMonsterData.generated.h"

class ARogueMonsterCorpse;
class URogueAction;


USTRUCT()
struct FHitReactConfig
{
	GENERATED_BODY()

	UAnimSequence* GetAnimFromAngle(AActor* HitActor, const FVector& ImpactDirection)
	{
		// 1.0 front, -1.0 is back
		float DotResult = FVector::DotProduct(HitActor->GetActorForwardVector(), ImpactDirection);
		if (DotResult <= -0.5f)
		{
			return FrontHit;
		}
		if (DotResult >= 0.5f)
		{
			return BackHit;
		}

		float SidewaysDotResult = FVector::DotProduct(HitActor->GetActorRightVector(), ImpactDirection);
		if (SidewaysDotResult <= -0.5f)
		{
			return RightHit;
		}
		if (SidewaysDotResult >= 0.5f)
		{
			return LeftHit;
		}

		// Fallback to front
		return FrontHit;
	}
	
	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	TObjectPtr<UAnimSequence> FrontHit;

	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	TObjectPtr<UAnimSequence> BackHit;
	
	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	TObjectPtr<UAnimSequence> LeftHit;
	
	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	TObjectPtr<UAnimSequence> RightHit;
	
	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	FName SlotName = "DefaultSlot";

	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	float BlendInTime = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	float BlendOutTime = 0.25f;
};


/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueMonsterData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Info")
	TSubclassOf<AActor> MonsterClass;

	/* Actions/buffs to grant this Monster */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Info")
	TArray<TSubclassOf<URogueAction>> Actions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, Category=Corpses)
	TSubclassOf<UAnimInstance> CorpseAnimInstance;

	/*
	 * Remaps Key to Value - Allow remapping bones to others in the skeleton to better handle impulses (for ragdolling)
	 */
	UPROPERTY(EditDefaultsOnly, Category=Physics)
	TMap<FName, FName> ImpulseBoneRemapping;

	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	FHitReactConfig HitReactions;
		
	/* Hitflash overlay material to use on receiving damage (used both the by the monster and the corpse instance */
	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	TObjectPtr<UMaterialInterface> HitFlashMaterial;
	
	/* Material Parameter that defines the time of hit */
	UPROPERTY(EditDefaultsOnly, Category=HitReactions)
	FName HitFlashTimeParamName = FName("TimeOfHit");

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("Monsters", GetFName());
	}
};
