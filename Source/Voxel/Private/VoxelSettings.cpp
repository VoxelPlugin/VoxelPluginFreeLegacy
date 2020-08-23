// Copyright 2020 Phyronnaz

#include "VoxelSettings.h"

UVoxelSettings::UVoxelSettings()
{
	CategoryName = "Plugins";
	SectionName = "Voxel Plugin";
}

FName UVoxelSettings::GetContainerName() const
{
	return "Project";
}

void UVoxelSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	if (IsTemplate())
	{
		ImportConsoleVariableValues();
	}
#endif
}

#if WITH_EDITOR
void UVoxelSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		ExportValuesToConsoleVariables(PropertyChangedEvent.Property);
	}
}
#endif