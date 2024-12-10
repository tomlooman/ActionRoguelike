// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"
#include "SGameplayInterface.h"
#include "DrawDebugHelpers.h"
#include "SWorldUserWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(SInteractionComponent)

namespace DebugDrawing
{
	static bool bDrawInteractionVisualize = false;
	static FAutoConsoleVariableRef CVarDebugDrawInteraction(TEXT("game.InteractionDebugDraw"),
		bDrawInteractionVisualize,
		TEXT("Enable Debug Lines for Interaction Component."),
		ECVF_Cheat);
}


USInteractionComponent::USInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Since we use Camera info in Tick we want the most up-to-date camera position for tracing
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	TraceRadius = 30.0f;
	TraceDistance = 500.0f;
	CollisionChannel = ECC_WorldDynamic;
}


void USInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Cast checked acts like static_cast in shipping builds. Less overhead compared to regular Cast<T> which does have safety nets.
	// Can use this in places where the cast object should never be nullptr by design and we know exactly the base class it is.
	const APawn* MyPawn = CastChecked<APawn>(GetOwner());
	if (MyPawn->IsLocallyControlled()) // Todo: Ideally just disable tick on this component when owner is not locally controlled.
	{
		FindBestInteractable();
	}
}


void USInteractionComponent::FindBestInteractable()
{
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);

	FVector EyeLocation;
	FRotator EyeRotation;
	GetOwner()->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	TArray<FHitResult> Hits;

	FCollisionShape Shape;
	Shape.SetSphere(TraceRadius);

	bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits, EyeLocation, TraceEnd, FQuat::Identity, ObjectQueryParams, Shape);

	FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;

	// Clear ref before trying to fill
	FocusedActor = nullptr;

	for (const FHitResult& Hit : Hits)
	{
		if (DebugDrawing::bDrawInteractionVisualize)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 32, LineColor, false, 0.0f);
		}

		if (AActor* HitActor = Hit.GetActor())
		{
			if (HitActor->Implements<USGameplayInterface>())
			{
				FocusedActor = HitActor;
				break;
			}
		}
	}

	if (FocusedActor)
	{
		if (WidgetInst == nullptr && ensure(DefaultWidgetClass))
		{
			WidgetInst = CreateWidget<USWorldUserWidget>(GetWorld(), DefaultWidgetClass);
		}

		if (WidgetInst)
		{
			WidgetInst->AttachedActor = FocusedActor;

			if (!WidgetInst->GetParent())
			{
				USWorldUserWidget::AddToRootCanvasPanel(WidgetInst);
			}
		}
	}
	else
	{
		if (WidgetInst)
		{
			WidgetInst->RemoveFromParent();
		}
	}


	if (DebugDrawing::bDrawInteractionVisualize)
	{
		DrawDebugLine(GetWorld(), EyeLocation, TraceEnd, LineColor, false, 2.0f, 0, 0.0f);
	}
}


void USInteractionComponent::PrimaryInteract()
{
	ServerInteract(FocusedActor);
}


void USInteractionComponent::ServerInteract_Implementation(AActor* InFocus)
{
	if (InFocus == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "No Focus Actor to interact.");
		return;
	}

	APawn* MyPawn = CastChecked<APawn>(GetOwner());
	ISGameplayInterface::Execute_Interact(InFocus, MyPawn);
}

