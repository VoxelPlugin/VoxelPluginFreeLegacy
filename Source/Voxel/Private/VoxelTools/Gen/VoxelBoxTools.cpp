// Copyright 2020 Phyronnaz

#include "VoxelTools/Gen/VoxelBoxTools.h"
#include "VoxelTools/Gen/VoxelGeneratedTools.h"
#include "VoxelTools/Impl/VoxelBoxToolsImpl.h"
#include "VoxelTools/Impl/VoxelBoxToolsImpl.inl"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelBoxTools::SetValueBox(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	float Value,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealValue = FVoxelValue(Value);
	
	GENERATED_TOOL_CALL(Value, FVoxelBoxToolsImpl::SetValueBox(Data, Bounds, RealValue));
}

void UVoxelBoxTools::SetValueBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	float Value,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealValue = FVoxelValue(Value);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelBoxToolsImpl::SetValueBox(Data, Bounds, RealValue));
}

void UVoxelBoxTools::SetValueBox(
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	float Value,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealValue = FVoxelValue(Value);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelBoxToolsImpl::SetValueBox(Data, Bounds, RealValue));
}

void UVoxelBoxTools::SetValueBoxAsync(
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	float Value,
	const FOnVoxelToolComplete_WithModifiedValues& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Value);
	
	const auto RealValue = FVoxelValue(Value);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelBoxToolsImpl::SetValueBox(Data, Bounds, RealValue));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelBoxTools::AddBox(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	GENERATED_TOOL_CALL(Value, FVoxelBoxToolsImpl::AddBox(Data, Bounds));
}

void UVoxelBoxTools::AddBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelBoxToolsImpl::AddBox(Data, Bounds));
}

void UVoxelBoxTools::AddBox(
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelBoxToolsImpl::AddBox(Data, Bounds));
}

void UVoxelBoxTools::AddBoxAsync(
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	const FOnVoxelToolComplete_WithModifiedValues& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Value);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelBoxToolsImpl::AddBox(Data, Bounds));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelBoxTools::RemoveBox(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	GENERATED_TOOL_CALL(Value, FVoxelBoxToolsImpl::RemoveBox(Data, Bounds));
}

void UVoxelBoxTools::RemoveBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelBoxToolsImpl::RemoveBox(Data, Bounds));
}

void UVoxelBoxTools::RemoveBox(
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelBoxToolsImpl::RemoveBox(Data, Bounds));
}

void UVoxelBoxTools::RemoveBoxAsync(
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	const FOnVoxelToolComplete_WithModifiedValues& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Value);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelBoxToolsImpl::RemoveBox(Data, Bounds));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelBoxTools::SetMaterialBox(
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	const FVoxelPaintMaterial& PaintMaterial,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Material);
	
	GENERATED_TOOL_CALL(Material, FVoxelBoxToolsImpl::SetMaterialBox(Data, Bounds, PaintMaterial));
}

void UVoxelBoxTools::SetMaterialBoxAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	const FVoxelPaintMaterial& PaintMaterial,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Material);
	
	GENERATED_TOOL_CALL_ASYNC(Material, FVoxelBoxToolsImpl::SetMaterialBox(Data, Bounds, PaintMaterial));
}

void UVoxelBoxTools::SetMaterialBox(
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	const FVoxelPaintMaterial& PaintMaterial,
	TArray<FModifiedVoxelMaterial>* OutModifiedMaterials,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Material);
	
	GENERATED_TOOL_CALL_CPP(Material, FVoxelBoxToolsImpl::SetMaterialBox(Data, Bounds, PaintMaterial));
}

void UVoxelBoxTools::SetMaterialBoxAsync(
	AVoxelWorld* VoxelWorld,
	const FVoxelIntBox& Bounds,
	const FVoxelPaintMaterial& PaintMaterial,
	const FOnVoxelToolComplete_WithModifiedMaterials& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Material);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Material, FVoxelBoxToolsImpl::SetMaterialBox(Data, Bounds, PaintMaterial));
}