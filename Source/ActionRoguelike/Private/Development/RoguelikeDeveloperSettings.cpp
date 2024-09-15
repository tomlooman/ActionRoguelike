// Fill out your copyright notice in the Description page of Project Settings.


#include "Development/RoguelikeDeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguelikeDeveloperSettings)

FName URoguelikeDeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}
