// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueDeferredTaskSystem.h"
#include "Containers/Queue.h"


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
}

TStatId URogueDeferredTaskSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(URogueDeferredTaskSystem, STATGROUP_Tickables);
}
