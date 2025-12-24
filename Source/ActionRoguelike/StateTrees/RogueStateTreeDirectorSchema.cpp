// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueStateTreeDirectorSchema.h"

#include "RogueStateTreeTasks.h"


bool URogueStateTreeDirectorSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	return Super::IsStructAllowed(InScriptStruct) ||
		InScriptStruct->IsChildOf(FRogueStateTreeDirectorTask::StaticStruct());
}