// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelHeightmapSamplerNode.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelNodeFunctions.h"

UVoxelNode_HeightmapSampler::UVoxelNode_HeightmapSampler()
{
	SetInputs(
		{ "X", EC::Float, "X between 0 and heightmap width" },
		{ "Y", EC::Float, "Y between 0 and heightmap height" });
	SetOutputs(
		{ "Height", EC::Float, "Height at position X Y" },
		{ "Material", EC::Material, "Material at position X Y" },
		{ "Min Height", EC::Float, "Min height of the entire heightmap" },
		{ "Max Height", EC::Float, "Max height of the entire heightmap" },
		{ "Size X", EC::Float, "Width of the heightmap. Affected by the asset XY Scale setting, so it may be a float" },
		{ "Size Y", EC::Float, "Height of the heightmap. Affected by the asset XY Scale setting, so it may be a float" });
}


FText UVoxelNode_HeightmapSampler::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Heightmap: {0}"), Super::GetTitle());
}

void UVoxelNode_HeightmapSampler::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	if ((bFloatHeightmap && !HeightmapFloat) || (!bFloatHeightmap && !HeightmapUINT16))
	{
		ErrorReporter.AddMessageToNode(this, "invalid heightmap", EVoxelGraphNodeMessageType::FatalError);
	}
}

#if WITH_EDITOR
bool UVoxelNode_HeightmapSampler::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	UVoxelHeightmapAsset* HeightmapAsset = nullptr;
	if (TryImportObject(Property, Object, HeightmapAsset))
	{
		if (bFloatHeightmap)
		{
			HeightmapFloat = Cast<UVoxelHeightmapAssetFloat>(HeightmapAsset);
		}
		else
		{
			HeightmapUINT16 = Cast<UVoxelHeightmapAssetUINT16>(HeightmapAsset);
		}
		return true;
	}
	return false;
}
#endif