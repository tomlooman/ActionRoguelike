// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoguePatrolPoint.generated.h"

UCLASS()
class ACTIONROGUELIKE_API ARoguePatrolPoint : public AActor
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY()
	TObjectPtr<USceneComponent> DefaultSceneRoot;
	
	/* read-only results in-editor, filled through ConstructScript */
	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<ARoguePatrolPoint>> NearbyPatrolPoints;
	
	UPROPERTY(EditAnywhere, Category="Patrol Points")
	float MaxPathLength = 1000.f;

public:
	
	TArray<ARoguePatrolPoint*> GetConnections() const
	{
		return NearbyPatrolPoints;
	}
	
	void RemovePatrolConnection(ARoguePatrolPoint* OtherPatrolPoint);
	
	void AddPatrolConnection(ARoguePatrolPoint* OtherPatrolPoint);
	
	virtual void OnConstruction(const FTransform& Transform) override;

	ARoguePatrolPoint();
};
