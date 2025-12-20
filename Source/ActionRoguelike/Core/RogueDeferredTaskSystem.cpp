// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueDeferredTaskSystem.h"
#include "Containers/Queue.h"


void URogueDeferredTaskSystem::AddTask(const UObject* WorldContextObject, TFunction<void()>&& InFunctionPtr)
{
	URogueDeferredTaskSystem* TaskSystem = WorldContextObject->GetWorld()->GetSubsystem<URogueDeferredTaskSystem>();
	TaskSystem->AddFunctionTask(MoveTemp(InFunctionPtr));
}


void URogueDeferredTaskSystem::AddFunctionTask(TFunction<void()>&& InFunctionPtr)
{
	FunctionPointers.Enqueue(MoveTemp(InFunctionPtr));
}


void URogueDeferredTaskSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TRACE_CPUPROFILER_EVENT_SCOPE(RogueDeferredTaskSystem::ProcessQueue);

	double StartTime = FPlatformTime::Seconds();
	const double BudgetSeconds = 0.002f;

	while (!FunctionPointers.IsEmpty())
	{
		if (FPlatformTime::Seconds() - StartTime > BudgetSeconds)
		{
			// Exceeded frame budget
			break;
		}
		
		TFunction<void()> Task;
		if (FunctionPointers.Dequeue(Task))
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(RogueDeferredTaskSystem::ProcessTask);
			// Run the lambda
			Task();
		}
	}
}

TStatId URogueDeferredTaskSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueDeferredTaskSystem, STATGROUP_Tickables);
}
