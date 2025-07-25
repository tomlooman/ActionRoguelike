// Fill out your copyright notice in the Description page of Project Settings.


#include "RoguePickupActor.h"

#include "ActionRoguelike.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/RoguePlayerCharacter.h"

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
	ARoguePlayerCharacter* MyPawn = Cast<ARoguePlayerCharacter>(OtherActor);
	// Skip for non-local pawns
	if (MyPawn && MyPawn->IsLocallyControlled())
	{
		Execute_Interact(this, CastChecked<AController>(MyPawn->GetController()));
	}
}


void ARoguePickupActor::Interact_Implementation(AController* InstigatorController)
{
	// logic in derived classes...
}


FText ARoguePickupActor::GetInteractText_Implementation(AController* InstigatorController)
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