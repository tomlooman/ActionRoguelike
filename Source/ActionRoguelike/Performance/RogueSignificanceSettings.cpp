// Fill out your copyright notice in the Description page of Project Settings.


#include "RogueSignificanceSettings.h"


namespace SignificanceManager
{
	/* Force significance on all classes to quickly compare the performance differences as if the system was disabled */
	static float ForcedSignificanceBucket = -1;
	static FAutoConsoleVariableRef CVarSignificanceManager_ForceBucket(
		TEXT("SigMan.ForceLOD"),
		ForcedSignificanceBucket,
		TEXT("Force bucket on all managed objects. -1 is default and means disabled, 0 is lowest bucket and means highest fidelity.\n"),
		ECVF_Cheat
		);

	/* Experimental: Resize the buckets based on Scalability setting. Could fit either in ViewDistance or Effect ScalabilityGroups */
	static float SignifanceBucketSizeMultiplier = 1.0;
	static FAutoConsoleVariableRef CVarSignificanceManager_BucketSizeMultiplier(
		TEXT("SigMan.SignificanceBucketSizeMultiplier"),
		SignifanceBucketSizeMultiplier,
		TEXT("Scalability multiplier to change the maximum number of Actors per Bucket.\n"),
		ECVF_Scalability
		);
}


int32 URogueSignificanceSettings::GetBucketIndex(FName InTag, int32 InIndex) const
{
	int32 TotalIndex = 0;

	// Development Only
	if (SignificanceManager::ForcedSignificanceBucket >= 0)
	{
		return SignificanceManager::ForcedSignificanceBucket;
	}

	const FSignificanceBucketInfo* BucketInfo = Buckets.Find(InTag);
	for (int BucketIndex = 0; BucketIndex < BucketInfo->BucketSizes.Num(); ++BucketIndex)
	{
		// Rounding down, not sure which option is best, requires more testing
		TotalIndex += FMath::Floor(BucketInfo->BucketSizes[BucketIndex] * SignificanceManager::SignifanceBucketSizeMultiplier);
		if (InIndex < TotalIndex)
		{
			return BucketIndex;
		}
	}

	// Returns the 'next' bucket (count from zero). With 4 buckets defined in Settings, this is bucket 5 with unlimited size.
	return BucketInfo->BucketSizes.Num();
}

FName URogueSignificanceSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}
