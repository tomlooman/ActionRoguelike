// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueTargetDummy.h"
#include "Components/StaticMeshComponent.h"
#include "ActionSystem/RogueAttributeComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueTargetDummy)


ARogueTargetDummy::ARogueTargetDummy()
{
	SkelMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = SkelMeshComp;

	AttributeComp = CreateDefaultSubobject<URogueAttributeComponent>(TEXT("AttributeComp"));
}

void ARogueTargetDummy::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Trigger when health is changed (damage/healing)
	AttributeComp->OnHealthChanged.AddDynamic(this, &ARogueTargetDummy::OnHealthChanged);
}


void ARogueTargetDummy::OnHealthChanged(AActor* InstigatorActor, URogueAttributeComponent* OwningComp, float NewHealth, float Delta)
{
	if (Delta < 0.0f)
	{
		SkelMeshComp->SetScalarParameterValueOnMaterials("TimeToHit", GetWorld()->TimeSeconds);
	}
}
