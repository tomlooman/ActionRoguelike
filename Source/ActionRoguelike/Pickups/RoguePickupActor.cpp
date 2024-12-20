// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor.h"

#include "ActionRoguelike.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePickupActor)


ARoguePickupActor::ARoguePickupActor()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetCollisionProfileName(Collision::Powerup_ProfileName);
	SphereComp->SetSphereRadius(100.0f);
	RootComponent = SphereComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	// Disable collision, instead we use SphereComp to handle interaction queries
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetupAttachment(RootComponent);

	RespawnTime = 10.0f;
	bIsActive = true;

	// Directly set bool instead of going through SetReplicates(true) within constructor,
	// Only use SetReplicates() outside constructor
	bReplicates = true;
}


void ARoguePickupActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Some pickups should auto pickup on overlap rather than a choice through player input
	if (bCanAutoPickup)
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	}
}

void ARoguePickupActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorldTimerManager().ClearAllTimersForObject(this);
}


void ARoguePickupActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                     int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// @todo: prevent minions from pickup coins. GameplayTags or collision channel.
	Execute_Interact(this, CastChecked<APawn>(OtherActor));
}


void ARoguePickupActor::Interact_Implementation(APawn* InstigatorPawn)
{
	// logic in derived classes...
}


FText ARoguePickupActor::GetInteractText_Implementation(APawn* InstigatorPawn)
{
	return FText::GetEmpty();
}


void ARoguePickupActor::ShowPickup()
{
	SetPickupState(true);
}


void ARoguePickupActor::HideAndCooldown()
{
	SetPickupState(false);

	FTimerHandle TimerHandle_RespawnTimer;
	GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ARoguePickupActor::ShowPickup, RespawnTime);
}

void ARoguePickupActor::SetPickupState(bool bNewIsActive)
{
	bIsActive = bNewIsActive;
	OnRep_IsActive();
}


void ARoguePickupActor::OnRep_IsActive()
{
	SetActorEnableCollision(bIsActive);
	// Set visibility on root and all children
	RootComponent->SetVisibility(bIsActive, true);
}


void ARoguePickupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARoguePickupActor, bIsActive);
}