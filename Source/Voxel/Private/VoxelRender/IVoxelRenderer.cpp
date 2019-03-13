// Copyright 2019 Phyronnaz

#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/VoxelIntermediateChunk.h"
#include "VoxelMaterialCollection.h"

#include "Logging/MessageLog.h"

#define LOCTEXT_NAMESPACE "Voxel"

UMaterialInterface* FVoxelRendererSettings::GetVoxelMaterialWithoutTessellation(const FVoxelBlendedMaterial& Index) const
{
	if (!MaterialCollection.IsValid())
	{
		FMessageLog("PIE").Error(LOCTEXT("InvalidMaterialCollection", "Invalid Material Collection"));
		return nullptr;
	}
	auto* Value = MaterialCollection->GetVoxelMaterial(Index);
	if (!Value)
	{
		FMessageLog("PIE").Error(FText::FromString(FString::Printf(TEXT("Missing the following generated material in %s: %s"), *MaterialCollection->GetPathName(), *Index.ToString())));
	}
	return Value;
}

UMaterialInterface* FVoxelRendererSettings::GetVoxelMaterialWithTessellation(const FVoxelBlendedMaterial& Index) const
{
	// TODO: Improve with fancy tokens & how to fix it
	if (!MaterialCollection.IsValid())
	{
		FMessageLog("PIE").Error(LOCTEXT("InvalidMaterialCollection", "Invalid Material Collection"));
		return nullptr;
	}
	auto* Value = MaterialCollection->GetVoxelMaterialWithTessellation(Index);
	if (!Value)
	{
		FMessageLog("PIE").Error(FText::FromString(FString::Printf(TEXT("Missing the following generated material in %s: Tessellation %s"), *MaterialCollection->GetPathName(), *Index.ToString())));
	}
	return Value;
}

UMaterialInterface* FVoxelRendererSettings::GetVoxelMaterialWithoutTessellation() const
{
	if (!VoxelMaterialWithoutTessellation.IsValid())
	{
		FMessageLog("PIE").Error(LOCTEXT("InvalidVoxelMaterial", "Invalid Voxel Material"));
		return nullptr;
	}
	return VoxelMaterialWithoutTessellation.Get();
}

UMaterialInterface* FVoxelRendererSettings::GetVoxelMaterialWithTessellation() const
{
	if (!VoxelMaterialWithTessellation.IsValid())
	{
		FMessageLog("PIE").Error(LOCTEXT("InvalidVoxelMaterialWithTessellation", "Invalid Tessellated Voxel Material"));
		return nullptr;
	}
	return VoxelMaterialWithTessellation.Get();
}

#undef LOCTEXT_NAMESPACE