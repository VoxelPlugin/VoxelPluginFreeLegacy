// Copyright 2020 Phyronnaz

#include "VoxelGraphBlueprintTools.h"
#include "VoxelWorldGenerator.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelGraphGenerator.h"
#include "VoxelMessages.h"

#define LOCTEXT_NAMESPACE "Voxel"

#define CHECK_WORLDGENERATOR() \
	if (!WorldGenerator) \
	{ \
		FVoxelMessages::Error(FUNCTION_ERROR("Invalid WorldGenerator!")); \
		return; \
	}

#define CANNOT_FIND_PROPERTY() \
			FVoxelMessages::Error( \
				FText::Format(LOCTEXT("CouldNotFindProperty", "{0}: Could not find property {1}!"), \
					FText::FromString(__FUNCTION__), \
					FText::FromName(UniqueName)));

void UVoxelGraphBlueprintTools::SetVoxelGraphFloatParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, float Value)
{
	VOXEL_PRO_ONLY_VOID();
}

void UVoxelGraphBlueprintTools::SetVoxelGraphIntParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, int32 Value)
{
	VOXEL_PRO_ONLY_VOID();
}

void UVoxelGraphBlueprintTools::SetVoxelGraphBoolParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, bool Value)
{
	VOXEL_PRO_ONLY_VOID();
}

void UVoxelGraphBlueprintTools::SetVoxelGraphColorParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, FLinearColor Value)
{
	VOXEL_PRO_ONLY_VOID();
}

void UVoxelGraphBlueprintTools::SetVoxelGraphVoxelTextureParameter(UVoxelWorldGenerator* WorldGenerator, FName UniqueName, FVoxelFloatTexture Value)
{
	VOXEL_PRO_ONLY_VOID();
}

void UVoxelGraphBlueprintTools::ClearVoxelGraphParametersOverrides(UVoxelWorldGenerator* WorldGenerator)
{
	VOXEL_PRO_ONLY_VOID();
}

#undef LOCTEXT_NAMESPACE