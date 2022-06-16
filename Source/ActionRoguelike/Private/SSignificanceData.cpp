// Fill out your copyright notice in the Description page of Project Settings.


#include "SSignificanceData.h"


float USSignificanceData::GetSignificanceByDistance(const FName& Tag, const float& DistanceSqrd) const
{
	// Find is hashed and will be plenty quick
	const TArray<FSignificanceThreshold> SignificanceThresholds = *SignificanceMap.Find(Tag);
	
	const int32 NumThresholds = SignificanceThresholds.Num();
	if (NumThresholds == 0)
	{
		// @todo: log warning
		return -1;
	}
	
	if (DistanceSqrd >= SignificanceThresholds[NumThresholds-1].MaxDistanceSqrd)
	{
		// Max distance reached
		return SignificanceThresholds[NumThresholds-1].Significance;
	}

	for (int32 Idx = 0; Idx < NumThresholds; Idx++)
	{
		const FSignificanceThreshold& Thresholds = SignificanceThresholds[Idx];
		if (DistanceSqrd <= Thresholds.MaxDistanceSqrd)
		{
			return Thresholds.Significance;
		}
	}

	// @todo: log warning
	return -1.f;
}


float USSignificanceData::GetMaxSignificance(FName Tag) const
{
	const TArray<FSignificanceThreshold> SignificanceThresholds = *SignificanceMap.Find(Tag);
	if (SignificanceThresholds.Num()>0)
	{
		return SignificanceThresholds[0].Significance;
	}

	return -1.f;
}


float USSignificanceData::GetSignificanceByThreshold(FName Tag, int32 Threshold) const
{
	const TArray<FSignificanceThreshold> SignificanceThresholds = *SignificanceMap.Find(Tag);
	if (SignificanceThresholds.IsValidIndex(Threshold))
	{
		return SignificanceThresholds[Threshold].Significance;
	}

	// @todo: warn about invalid setup
	return -1.f;
}


void USSignificanceData::RefreshCachedData()
{
	// Convert the Curves into simple Arrays with thresholds (eg. significance 0, 1, 10 where each Key on the curve is one threshold)
	SignificanceMap.Reset();
	for (auto Item : SignificanceCurves)
	{
		// Refresh the data for quick access by significance managed objects
		int32 NrKeys = Item.Value.DistanceCurve.GetRichCurveConst()->Keys.Num();
		TArray<FSignificanceThreshold> Thresholds;
		Thresholds.Reserve(NrKeys);
		
		// Add every key, acts as 'threshold' (eg. 2.0 Time = 2.0 Significance, and Value = DistanceTo
		for (FRichCurveKey Key : Item.Value.DistanceCurve.GetRichCurveConst()->Keys)
		{
			Thresholds.Emplace(Key.Time, Key.Value);
		}

		// Reverse so last entry in array is 'highest significance'
		Algo::Reverse(Thresholds);

		SignificanceMap.Emplace(Item.Key, Thresholds);
	}
}


void USSignificanceData::Serialize(FStructuredArchiveRecord Record)
{
	// Refresh thresholds used by game code when tweaking at runtime and/or in editor
	RefreshCachedData();
	Super::Serialize(Record);
}


void USSignificanceData::PostLoad()
{
	// Refresh thresholds used by game code when first loaded
	Super::PostLoad();
	RefreshCachedData();
}