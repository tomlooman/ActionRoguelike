// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SSignificanceData.generated.h"

struct FSignificanceThreshold
{
	FSignificanceThreshold(float InSignificance, float InMaxDistance) :
		Significance(InSignificance),
		MaxDistanceSqrd(InMaxDistance*InMaxDistance) {}

	float Significance;

	// Dist squared for perf while keeping frontend easy for users in unreal units
	float MaxDistanceSqrd;
};

/* Provides significance config per Type (SignificanceTag) */
USTRUCT(BlueprintType)
struct FSignificanceItem
{
	GENERATED_BODY()

	FSignificanceItem() :
		ForceSingificance(-1.0f) {}

	UPROPERTY(EditAnywhere)
	FRuntimeFloatCurve DistanceCurve;

	UPROPERTY(EditAnywhere, meta = (AdvancedDisplay))
	float ForceSingificance;
};

/**
 * Holds user configured data per SignificanceTag (eg. NPC, Player, TreasureChest), can be tweaked at runtime.
 */
UCLASS(Config="Game")
class ACTIONROGUELIKE_API USSignificanceData : public UDataAsset
{
	GENERATED_BODY()
	
	virtual void Serialize(FStructuredArchiveRecord Record) override;

	virtual void PostLoad() override;

	void RefreshCachedData();

public:

	float GetSignificanceByDistance(const FName& Tag, const float& DistanceSqrd) const;

	float GetMaxSignificance(FName Tag) const;

	float GetSignificanceByThreshold(FName Tag, int32 Threshold) const;

	/* Define a Curve per class, eg. "NPC" or a specific instance "NPC_Cinematic01", these tags are specified per class when they register with the manager */
	UPROPERTY(EditAnywhere)
	TMap<FName, FSignificanceItem> SignificanceCurves;

	// Cached map for quick access and distinct thresholds rater than Eval() on curve
	TMap<FName, TArray<FSignificanceThreshold>> SignificanceMap;
};
