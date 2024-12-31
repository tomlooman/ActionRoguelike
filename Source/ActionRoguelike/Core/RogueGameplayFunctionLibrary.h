// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RogueGameplayFunctionLibrary.generated.h"


USTRUCT()
struct FDamageInfo
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> DamageInstigator;

	float AttackDamage;
};

/**
 * 
 */
UCLASS()
class ACTIONROGUELIKE_API URogueGameplayFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category=Gameplay)
	static URogueActionComponent* GetActionComponentFromActor(AActor* FromActor);

	UFUNCTION(BlueprintCallable, Category=Gameplay)
	static bool IsAlive(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category=Gameplay)
	static bool ApplyDamage(AActor* DamageCauser, AActor* TargetActor, float DamageCoefficient);

	UFUNCTION(BlueprintCallable, Category=Gameplay)
	static bool ApplyDirectionalDamage(AActor* DamageCauser, AActor* TargetActor, float DamageCoefficient, const FHitResult& HitResult);
/*
	static bool ApplyRadialDamage(AActor* DamageCauser, FVector Origin, float DamageRadius, float DamageCoefficient);
*/
	//void OnDamageOverlapComplete(const FTraceHandle& TraceHandle, FOverlapDatum& OverlapDatum, FDamageInfo DamageInfo);
	
	UFUNCTION(BlueprintPure, Category = "PSO Caching")
	static int32 GetRemainingBundledPSOs();
};
