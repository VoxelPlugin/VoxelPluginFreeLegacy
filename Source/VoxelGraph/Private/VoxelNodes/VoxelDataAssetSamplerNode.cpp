// Copyright 2021 Phyronnaz

#include "VoxelNodes/VoxelDataAssetSamplerNode.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.inl"

UVoxelNode_DataAssetSampler::UVoxelNode_DataAssetSampler()
{
	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" });
	SetOutputs(
		{ "Value", EC::Float, "Data asset value at position X Y Z. Between -1 and 1." },
		{ "Material", EC::Material, "Data asset material at position X Y Z" },
		{ "Size X", EC::Int, "Size of the data asset" },
		{ "Size Y", EC::Int, "Size of the data asset" },
		{ "Size Z", EC::Int, "Size of the data asset" });
}


EVoxelPinCategory UVoxelNode_DataAssetSampler::GetInputPinCategory(int32 PinIndex) const
{
	return bBilinearInterpolation ? EC::Float : EC::Int;
}

FText UVoxelNode_DataAssetSampler::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Data Asset: {0}"), Super::GetTitle());
}