// Fill out your copyright notice in the Description page of Project Settings.


#include "SSignificanceManagerSettings.h"

USSignificanceManagerSettings::USSignificanceManagerSettings()
{
	bEnableSingificanceManager = true;
}


USSignificanceData* USSignificanceManagerSettings::GetConfig() const
{
	// Loads if needed, otherwise returns laoded asset
	return ConfigurationAsset.LoadSynchronous();
}
