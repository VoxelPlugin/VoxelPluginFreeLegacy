// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"
#include "VoxelGraphGlobals.h"
#include "VoxelUtilities/VoxelRangeUtilities.h"

class FVoxelPlaceableItemHolder;

DEPRECATED_VOXEL_GRAPH_FUNCTION()
typedef EVoxelDataItemCombineMode EVoxelDataItemSampleCombineMode;

namespace FVoxelNodeFunctions
{
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline v_flt GetPerlinWormsDistance(const FVoxelPlaceableItemHolder& ItemHolder, v_flt X, v_flt Y, v_flt Z)
	{
		return 0;
	}
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline TVoxelRange<v_flt> GetPerlinWormsDistance(const FVoxelPlaceableItemHolder& ItemHolder, const TVoxelRange<v_flt>& X, const TVoxelRange<v_flt>& Y, const TVoxelRange<v_flt>& Z)
	{
		return 0;
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline FVoxelIntBox BoundsFromRanges(TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z)
	{
		return FVoxelRangeUtilities::BoundsFromRanges(X, Y, Z);
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline v_flt GetWorldGeneratorCustomOutput(
		const FVoxelGeneratorInstance& Generator,
		const FName& Name,
		v_flt X, v_flt Y, v_flt Z,
		const FVoxelContext& Context)
	{
		return GetGeneratorCustomOutput(Generator, Name, X, Y, Z, Context);
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline TVoxelRange<v_flt> GetWorldGeneratorCustomOutput(
		const FVoxelGeneratorInstance& Generator,
		const FName& Name,
		TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, TVoxelRange<v_flt> Z,
		const FVoxelContextRange& Context)
	{
		return GetGeneratorCustomOutput(Generator, Name, X, Y, Z, Context);
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>> CreateWorldGeneratorArray(const TArray<FVoxelGeneratorPicker>& Generators)
	{
		return CreateGeneratorArray(Generators);
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline void ComputeWorldGeneratorsMerge(
		EVoxelMaterialConfig MaterialConfig,
		float Tolerance,
		const TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>>& InInstances,
		const TArray<FName>& FloatOutputsNames,
		const FVoxelContext& Context,
		v_flt X, v_flt Y, v_flt Z,
		int32 Index0, float Alpha0,
		int32 Index1, float Alpha1,
		int32 Index2, float Alpha2,
		int32 Index3, float Alpha3,
		bool bComputeValue, bool bComputeMaterial, const TArray<bool>& ComputeFloatOutputs,
		v_flt& OutValue,
		FVoxelMaterial& OutMaterial,
		TArray<v_flt, TInlineAllocator<128>>& OutFloatOutputs,
		int32& NumGeneratorsQueried)
	{
	}
	
	DEPRECATED_VOXEL_GRAPH_FUNCTION()
	inline void ComputeWorldGeneratorsMergeRange(
		const TArray<TVoxelSharedPtr<FVoxelGeneratorInstance>>& InInstances,
		const TArray<FName>& FloatOutputsNames,
		const FVoxelContextRange& Context,
		TVoxelRange<v_flt> X,
		TVoxelRange<v_flt> Y,
		TVoxelRange<v_flt> Z,
		bool bComputeValue, const TArray<bool>& ComputeFloatOutputs,
		TVoxelRange<v_flt>& OutValue,
		TArray<TVoxelRange<v_flt>, TInlineAllocator<128>> & OutFloatOutputs,
		TVoxelRange<int32>& NumGeneratorsQueried)
	{
	}
}