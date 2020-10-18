// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class UMaterial;

struct VOXEL_API FVoxelEditorDelegates
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FFixVoxelLandscapeMaterial, UMaterial*);
	static FFixVoxelLandscapeMaterial FixVoxelLandscapeMaterial;
};