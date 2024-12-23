// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueAnimNotifyState_Melee.h"

#include "Engine/OverlapResult.h"


URogueAnimNotifyState_Melee::URogueAnimNotifyState_Melee()
{
	Radius = 128.0f;
	TraceChannel = ECC_Pawn;
}


void URogueAnimNotifyState_Melee::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	// We could run async requests here continuously any time we have new results to fetch, could stop request after the first successful 'hit'

	FVector StartLoc = MeshComp->GetSocketLocation(SocketName);
	// assert on invalid bone name, or some other type of error throwing

	UWorld* World = MeshComp->GetWorld();

	// first impl 1 is just synchronous (todo: make this async)
	TArray<FOverlapResult> Overlaps;

	FCollisionShape Shape;
	Shape.SetSphere(Radius);

	DrawDebugSphere(World, StartLoc, Radius, 32.f, FColor::Cyan, false, 0.0f);

	World->OverlapMultiByChannel(Overlaps, StartLoc, FQuat::Identity, TraceChannel, Shape);

	for (FOverlapResult& Result : Overlaps)
	{
		DrawDebugBox(World, Result.GetComponent()->GetComponentLocation(), FVector(32.f), FColor::Green, false, 5.0f);
	}
}
