// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UMaterial;

struct VOXEL_API FVoxelEditorDelegates
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FFixVoxelLandscapeMaterial, UMaterial*);
	static FFixVoxelLandscapeMaterial FixVoxelLandscapeMaterial;
};