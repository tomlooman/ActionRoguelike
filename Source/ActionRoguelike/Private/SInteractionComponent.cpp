// Fill out your copyright notice in the Description page of Project Settings.


#include "SInteractionComponent.h"

#include "ActionRoguelike.h"
#include "SGameplayInterface.h"
#include "DrawDebugHelpers.h"
#include "SWorldUserWidget.h"
#include "Engine/OverlapResult.h"

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

	TraceRadius = 250.f;
	TraceChannel = TRACE_INTERACT;
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
	UWorld* World = GetWorld();

	FVector TraceOrigin = GetOwner()->GetActorLocation();

	// Find all potential interactables around the player
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(
		Overlaps,
		TraceOrigin,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius));
	
	FColor LineColor = FColor::Green;
	if (DebugDrawing::bDrawInteractionVisualize)
	{
		DrawDebugSphere(World, TraceOrigin, TraceRadius, 32, LineColor, false, 0.0f);
	}

	APawn* OwningPawn = Cast<APawn>(GetOwner());
	AController* OwningController = OwningPawn->GetController();
	check(OwningController); // We already check if locally controlled earlier
	
	FocusedActor = nullptr;
	float HighestWeight = -MAX_flt;

	// Calc 'weights' to find the best interactable which the player most likely intends to focus
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AActor* HitActor = Overlap.GetActor())
		{
			if (DebugDrawing::bDrawInteractionVisualize)
			{
				DrawDebugSphere(GetWorld(), HitActor->GetActorLocation(),
					32, 16, LineColor, false, 0.0f);
			}
			
			if (HitActor->Implements<USGameplayInterface>())
			{
				// When not recently rendered, the player is unlikely to want to interact with this object
				// it might be hidden behind walls (shadow rendering might mess with this boolean)
				if (!HitActor->WasRecentlyRendered(0.0f))
				{
					// Won't work for Nanite meshes which don't report this recently rendered
					// - Extra: if recently rendered, then perform line trace from camera to obj pivot
					//				to know if we are actually behind some type of wall
					continue;
				}

				FVector ObjectDir = (HitActor->GetActorLocation() - TraceOrigin).GetUnsafeNormal();

				// Prefer actors that our 'control rotation' (eg. camera) is pointing to
				float DotResult = FVector::DotProduct(ObjectDir, OwningController->GetControlRotation().Vector());
				float Weight = DotResult * 10.f;
				
				if (HighestWeight < Weight)
				{
					FocusedActor = HitActor;
					HighestWeight = Weight;
				}
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
		if (FocusedActor)
		{
			DrawDebugBox(GetWorld(), FocusedActor->GetActorLocation(), FVector(20.f),
				LineColor, false, 0.0f);
		}
		//DrawDebugLine(GetWorld(), TraceOrigin, TraceEnd, LineColor, false, 2.0f, 0, 0.0f);
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

