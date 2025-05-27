// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueInteractionComponent.h"

#include "ActionRoguelike.h"
#include "Core/RogueGameplayInterface.h"
#include "DrawDebugHelpers.h"
#include "Core/RogueGameplayFunctionLibrary.h"
#include "UI/RogueWorldUserWidget.h"
#include "Engine/OverlapResult.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueInteractionComponent)

namespace DebugDrawing
{
	static bool bDrawInteractionVisualize = false;
	static FAutoConsoleVariableRef CVarDebugDrawInteraction(TEXT("game.InteractionDebugDraw"),
		bDrawInteractionVisualize,
		TEXT("Enable Debug Lines for Interaction Component."),
		ECVF_Cheat);
}


URogueInteractionComponent::URogueInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// Since we use Camera info in Tick we want the most up-to-date camera position for tracing
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	TraceRadius = 250.f;
	TraceChannel = TRACE_INTERACT;
}


void URogueInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// Cast checked acts like static_cast in shipping builds. Less overhead compared to regular Cast<T> which does have safety nets.
	// Can use this in places where the cast object should never be nullptr by design, and we know exactly the base class it is.
	const AController* MyController = CastChecked<AController>(GetOwner());
	if (MyController->IsLocalController())
	{
		FindBestInteractable();
	}
}


void URogueInteractionComponent::FindBestInteractable()
{
	AController* OwningController = Cast<AController>(GetOwner());
	check(OwningController); // Only allow this to exist on (player)controllers
	
	APawn* OwningPawn = OwningController->GetPawn();
	if (OwningPawn == nullptr || !URogueGameplayFunctionLibrary::IsAlive(OwningPawn))
	{
		// We do not always have a pawn, or we died
		FocusedActor = nullptr;
		return;
	}

	UWorld* World = GetWorld();
	const FVector TraceOrigin = OwningPawn->GetActorLocation();
	const FColor DebugLineColor = FColor::Green;
	
	// Find all potential interactable around the player
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(
		Overlaps,
		TraceOrigin,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius));
	

	if (DebugDrawing::bDrawInteractionVisualize)
	{
		DrawDebugSphere(World, TraceOrigin, TraceRadius, 32, DebugLineColor, false, 0.0f);
	}

	// Reset
	FocusedActor = nullptr;
	float HighestWeight = -MAX_flt;

	// Calc 'weights' to find the best interactable which the player most likely intends to focus
	for (const FOverlapResult& Overlap : Overlaps)
	{
		if (AActor* HitActor = Overlap.GetActor())
		{
			if (DebugDrawing::bDrawInteractionVisualize)
			{
				DrawDebugSphere(World, HitActor->GetActorLocation(),
					32, 16, DebugLineColor, false, 0.0f);
			}
			
			if (HitActor->Implements<URogueGameplayInterface>())
			{
				// When not recently rendered, the player is unlikely to want to interact with this object
				// it might be hidden behind walls (shadow rendering might mess with this boolean)
				//if (!HitActor->WasRecentlyRendered(0.1f))
				{
					// Won't work for Nanite meshes which don't report this recently rendered
					// - Extra: if recently rendered, then perform line trace from camera to obj pivot
					//				to know if we are actually behind some type of wall
					//continue;
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
		if (WidgetInst == nullptr && ensureMsgf(DefaultWidgetClass, TEXT("DefaultWidgetClass for the interaction component is not specified in %s. Please update the Blueprint."), *GetNameSafe(OwningController)))
		{
			WidgetInst = CreateWidget<URogueWorldUserWidget>(World, DefaultWidgetClass);
		}

		if (WidgetInst)
		{
			WidgetInst->AttachedActor = FocusedActor;

			if (!WidgetInst->GetParent())
			{
				URogueWorldUserWidget::AddToRootCanvasPanel(WidgetInst);
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
			DrawDebugBox(World, FocusedActor->GetActorLocation(), FVector(20.f),
				DebugLineColor, false, 0.0f);
		}
		//DrawDebugLine(World, TraceOrigin, TraceEnd, LineColor, false, 2.0f, 0, 0.0f);
	}
}


void URogueInteractionComponent::PrimaryInteract()
{
	ServerInteract(FocusedActor);
}


void URogueInteractionComponent::ServerInteract_Implementation(AActor* InFocus)
{
	if (InFocus == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, "No Focus Actor to interact.");
		return;
	}

	AController* MyController = CastChecked<AController>(GetOwner());
	IRogueGameplayInterface::Execute_Interact(InFocus, MyController);
}

