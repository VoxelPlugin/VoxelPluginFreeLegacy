// Copyright 2020 Phyronnaz

#include "VoxelGraphBlueprintTools.h"
#include "VoxelWorldGenerator.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelGraphGenerator.h"
#include "VoxelMessages.h"

#define CHECK_WORLDGENERATOR() \
	if (!WorldGenerator) \
	{ \
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid WorldGenerator!")); \
		return false; \
	}

#define CANNOT_FIND_PROPERTY() \
			FVoxelMessages::Error( \
				FText::Format(VOXEL_LOCTEXT("{0}: Could not find property {1}!"), \
					FText::FromString(__FUNCTION__), \
					FText::FromName(UniqueName))); \
			return false;

bool UVoxelGraphBlueprintTools::SetVoxelGraphFloatParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, float Value)
{
	VOXEL_PRO_ONLY();
}

bool UVoxelGraphBlueprintTools::SetVoxelGraphIntParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, int32 Value)
{
	VOXEL_PRO_ONLY();
}

bool UVoxelGraphBlueprintTools::SetVoxelGraphBoolParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, bool Value)
{
	VOXEL_PRO_ONLY();
}

bool UVoxelGraphBlueprintTools::SetVoxelGraphColorParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, FLinearColor Value)
{
	VOXEL_PRO_ONLY();
}

bool UVoxelGraphBlueprintTools::SetVoxelGraphVoxelTextureParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, FVoxelFloatTexture Value)
{
	VOXEL_PRO_ONLY();
}

void UVoxelGraphBlueprintTools::ClearVoxelGraphParametersOverrides(UVoxelWorldGenerator* WorldGenerator)
{
	VOXEL_PRO_ONLY_VOID();
}