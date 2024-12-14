// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SignificanceManager.h"
#include "Components/ActorComponent.h"
#include "SSignificanceComponent.generated.h"


UENUM(BlueprintType)
enum class ESignificanceValue : uint8
{
	// Special tier while owning Actor is hidden
	Hidden = 0, 
	// Assumed very far away
	Lowest = 1, 
	// no longer critical
	Medium = 2,
	// Max fidelity, No throttling/culling
	Highest = 3,
	// not yet defined, good starting position
	Invalid = UINT8_MAX 
};

// Trigger only when significance has changed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostSignificanceChanged, ESignificanceValue, Significance);

/* Configure mapping between Significance and Distance to. While distance from viewpoint is within "MaxDistance" the specified "Significance" will be used. */
USTRUCT(BlueprintType)
struct FSignificanceDistance
{
	GENERATED_BODY()

	FSignificanceDistance()
	{
		Significance = ESignificanceValue::Highest;
		MaxDistance = 1000.f;
	}
	
	FSignificanceDistance(ESignificanceValue InSignificance, float InMaxDistance) :
		Significance(InSignificance),
		MaxDistance(InMaxDistance) {}

	UPROPERTY(EditAnywhere)
	ESignificanceValue Significance;

	UPROPERTY(EditAnywhere, meta = (UIMin=1000.0, UIMax=50000))
	float MaxDistance;

	float GetMaxDistSqrd() const { return MaxDistance*MaxDistance; }
};

/*
 * Significance Component made entirely for convenience and classes that don't have a C++ Actor base class.
 * To squeeze max. perf out of signifance you may want to skip this component and directly implement the Significance directly as this component essentially demonstrates how.
 * See: RegisterWithManager() on which functions are required to hook yourself up with SignifanceManager.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), HideCategories=("Collision", "Tags", "ComponentTick", "ComponentReplication", "Cooking", "Activation", "AssetUserData") )
class ACTIONROGUELIKE_API USSignificanceComponent : public UActorComponent
{
	GENERATED_BODY()
	
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	
	/* Register with Significance Manager to optimize ticks/logic (Allows per (child)class / instance opt-out) */
	UPROPERTY(EditAnywhere, Category = "Optimization")
	bool bManageSignificance;
	
	/* MaxDistance per Significance, defaults to 0.0 (lowest) Significance if outside bounds of distances specified here  */
	UPROPERTY(EditAnywhere, meta = (TitleProperty="MaxDistance"))
	TArray<FSignificanceDistance> Thresholds;

	UPROPERTY(Transient, BlueprintReadOnly, Category= "Optimization")
	ESignificanceValue CurrentSignificance;

protected:
	
	/* Considered insignificant (-1.0) while owning Actor is HiddenInGame */
	UPROPERTY(EditAnywhere, Category = "Optimization")
	bool bInsignificantWhenOwnerIsHidden;
	
	/* Delay registration by one tick to allow actors/components to tick once, this may be useful to have them setup VFX beams etc. before pausing them due to low significance */
	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	bool bWaitOneFrame;

	UPROPERTY(EditDefaultsOnly, Category = "Optimization")
	bool bManageOwnerParticleSignificance;

	bool bHasImplementedInterface;

	UFUNCTION()
	void RegisterWithManager();

	float CalcSignificance(USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& Viewpoint) const;
	
	void PostSignificanceUpdate(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSignificance, float Significance, bool bFinal);

	float GetSignificanceByDistance(float DistanceSqrd) const;

	void UpdateParticleSignificance(float NewSignificance);

public:
	
	UPROPERTY(BlueprintAssignable)
	FOnPostSignificanceChanged OnSignificanceChanged;

	USSignificanceComponent();
};
