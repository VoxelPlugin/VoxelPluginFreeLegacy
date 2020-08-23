// Copyright 2020 Phyronnaz

#include "VoxelTools/Gen/VoxelSurfaceEditTools.h"
#include "VoxelTools/Gen/VoxelGeneratedTools.h"
#include "VoxelTools/Impl/VoxelSurfaceEditToolsImpl.h"
#include "VoxelTools/Impl/VoxelSurfaceEditToolsImpl.inl"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceEditTools::EditVoxelValues(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	float DistanceDivisor,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	const FVoxelHardnessHandler HardnessHandler(*VoxelWorld);
	
	GENERATED_TOOL_CALL(Value, FVoxelSurfaceEditToolsImpl::EditVoxelValues(Data, HardnessHandler, Bounds, ProcessedVoxels, DistanceDivisor));
}

void UVoxelSurfaceEditTools::EditVoxelValuesAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	float DistanceDivisor,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	const FVoxelHardnessHandler HardnessHandler(*VoxelWorld);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSurfaceEditToolsImpl::EditVoxelValues(Data, HardnessHandler, Bounds, ProcessedVoxels, DistanceDivisor));
}

void UVoxelSurfaceEditTools::EditVoxelValues(
	AVoxelWorld* VoxelWorld,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	float DistanceDivisor,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	const FVoxelHardnessHandler HardnessHandler(*VoxelWorld);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSurfaceEditToolsImpl::EditVoxelValues(Data, HardnessHandler, Bounds, ProcessedVoxels, DistanceDivisor));
}

void UVoxelSurfaceEditTools::EditVoxelValuesAsync(
	AVoxelWorld* VoxelWorld,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	float DistanceDivisor,
	const FOnVoxelToolComplete_WithModifiedValues& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Value);
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	const FVoxelHardnessHandler HardnessHandler(*VoxelWorld);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSurfaceEditToolsImpl::EditVoxelValues(Data, HardnessHandler, Bounds, ProcessedVoxels, DistanceDivisor));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceEditTools::EditVoxelMaterials(
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelPaintMaterial& PaintMaterial,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Material);
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	
	GENERATED_TOOL_CALL(Material, FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(Data, Bounds, PaintMaterial, ProcessedVoxels));
}

void UVoxelSurfaceEditTools::EditVoxelMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelPaintMaterial& PaintMaterial,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Material);
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	
	GENERATED_TOOL_CALL_ASYNC(Material, FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(Data, Bounds, PaintMaterial, ProcessedVoxels));
}

void UVoxelSurfaceEditTools::EditVoxelMaterials(
	AVoxelWorld* VoxelWorld,
	const FVoxelPaintMaterial& PaintMaterial,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	TArray<FModifiedVoxelMaterial>* OutModifiedMaterials,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Material);
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	
	GENERATED_TOOL_CALL_CPP(Material, FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(Data, Bounds, PaintMaterial, ProcessedVoxels));
}

void UVoxelSurfaceEditTools::EditVoxelMaterialsAsync(
	AVoxelWorld* VoxelWorld,
	const FVoxelPaintMaterial& PaintMaterial,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	const FOnVoxelToolComplete_WithModifiedMaterials& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Material);
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Material, FVoxelSurfaceEditToolsImpl::EditVoxelMaterials(Data, Bounds, PaintMaterial, ProcessedVoxels));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSurfaceEditTools::PropagateVoxelMaterials(
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Material);
	
	if (!ProcessedVoxels.Info.bHasSurfacePositions)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("PropagateVoxelMaterials needs surface positions! Use FindSurfaceVoxelsFromDistanceField"));
		return;
	}
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	
	GENERATED_TOOL_CALL(Material, FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials(Data, ProcessedVoxels));
}

void UVoxelSurfaceEditTools::PropagateVoxelMaterialsAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Material);
	
	if (!ProcessedVoxels.Info.bHasSurfacePositions)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("PropagateVoxelMaterials needs surface positions! Use FindSurfaceVoxelsFromDistanceField"));
		return;
	}
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	
	GENERATED_TOOL_CALL_ASYNC(Material, FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials(Data, ProcessedVoxels));
}

void UVoxelSurfaceEditTools::PropagateVoxelMaterials(
	AVoxelWorld* VoxelWorld,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	TArray<FModifiedVoxelMaterial>* OutModifiedMaterials,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Material);
	
	if (!ProcessedVoxels.Info.bHasSurfacePositions)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("PropagateVoxelMaterials needs surface positions! Use FindSurfaceVoxelsFromDistanceField"));
		return;
	}
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	
	GENERATED_TOOL_CALL_CPP(Material, FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials(Data, ProcessedVoxels));
}

void UVoxelSurfaceEditTools::PropagateVoxelMaterialsAsync(
	AVoxelWorld* VoxelWorld,
	const FVoxelSurfaceEditsProcessedVoxels& ProcessedVoxels,
	const FOnVoxelToolComplete_WithModifiedMaterials& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Material);
	
	if (!ProcessedVoxels.Info.bHasSurfacePositions)
	{
		FVoxelMessages::Error(FUNCTION_ERROR("PropagateVoxelMaterials needs surface positions! Use FindSurfaceVoxelsFromDistanceField"));
		return;
	}
	
	if (!FVoxelSurfaceEditToolsImpl::ShouldCompute(ProcessedVoxels))
	{
		return;
	}
	
	const FVoxelIntBox Bounds = FVoxelSurfaceEditToolsImpl::GetBounds(ProcessedVoxels);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Material, FVoxelSurfaceEditToolsImpl::PropagateVoxelMaterials(Data, ProcessedVoxels));
}