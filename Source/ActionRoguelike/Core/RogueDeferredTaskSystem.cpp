// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueDeferredTaskSystem.h"
#include "Containers/Queue.h"


void URogueDeferredTaskSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Track the frame start time to know if we have budget to process more this frame
	FCoreDelegates::OnBeginFrame.AddLambda([&]
	{
		FrameStartTime = FPlatformTime::Seconds();
	});
}

void URogueDeferredTaskSystem::AddLambda(const UObject* WorldContextObject, TFunction<void()> InFunctionPtr)
{
	URogueDeferredTaskSystem* TaskSystem = WorldContextObject->GetWorld()->GetSubsystem<URogueDeferredTaskSystem>();
	TaskSystem->AddFunction(MoveTemp(InFunctionPtr));
}


void URogueDeferredTaskSystem::AddFunction(TFunction<void()> InFunctionPtr)
{
	FDeferredTask NewTask = FDeferredTask();
	NewTask.FunctionPtr = MoveTemp(InFunctionPtr);
	
	FunctionPointers.Enqueue(NewTask);
}


void URogueDeferredTaskSystem::AddDelegate(FDeferredTaskDelegate InDelegate)
{
	FDeferredTask NewTask = FDeferredTask();
	NewTask.Delegate = InDelegate;
	
	FunctionPointers.Enqueue(NewTask);
}


void URogueDeferredTaskSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if USE_DEFERRED_TASKS
	
	TRACE_CPUPROFILER_EVENT_SCOPE(RogueDeferredTaskSystem::ProcessQueue);

	double QueueStartTime = FPlatformTime::Seconds();
	// Allow at least this much time to be spent during a frame to avoid stalling the queue
	const double MinBudgetSeconds = 0.001f; // 1ms
	// Target budget to stay under each frame
	constexpr float TargetFPS = 120;
	constexpr float MaxFrameBudget = 1.0f / TargetFPS;
	
	//UE_LOG(LogTemp, Log, TEXT("Frame time so far: %f"), (QueueStartTime - FrameStartTime));

	while (!FunctionPointers.IsEmpty())
	{
		// @todo: issue remains that we don't process this subsystem at the end of the frame so we need to
		// keep time available for other systems including Slate to still take up more GameThread time.
		// this is experimental, the primary element of simply having a per-frame budget is the simplest and flattest approach to handling this.
		double TotalFrameTime = FPlatformTime::Seconds() - FrameStartTime;

		if (TotalFrameTime >= MaxFrameBudget)
		{
			// Exceeded out budget but forcefully continue until we hit the min time per frame to spend.
			if (FPlatformTime::Seconds() - QueueStartTime > MinBudgetSeconds)
			{
				// Exceeded frame budget
				break;
			}
		}
		
		FDeferredTask Task;
		if (FunctionPointers.Dequeue(Task))
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(RogueDeferredTaskSystem::ProcessTask);
			// Run the lambda
			if (Task.FunctionPtr.IsSet())
			{
				Task.FunctionPtr();
			}
			// Run optional delegate (for blueprint)
			Task.Delegate.ExecuteIfBound();
		}
	}

#endif
}
