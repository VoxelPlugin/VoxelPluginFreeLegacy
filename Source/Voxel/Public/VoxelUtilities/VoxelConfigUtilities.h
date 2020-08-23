// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

namespace FVoxelConfigUtilities
{
	VOXEL_API void SaveConfig(UObject* Object, const FString& BaseSectionName = "Voxel", const FString& Filename = GEditorPerProjectIni);
	VOXEL_API void LoadConfig(UObject* Object, const FString& BaseSectionName = "Voxel", const FString& Filename = GEditorPerProjectIni);
}