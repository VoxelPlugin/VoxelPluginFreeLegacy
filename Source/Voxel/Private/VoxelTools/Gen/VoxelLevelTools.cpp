// Copyright 2020 Phyronnaz

#include "VoxelTools/Gen/VoxelLevelTools.h"
#include "VoxelTools/Gen/VoxelGeneratedTools.h"
#include "VoxelTools/Impl/VoxelLevelToolsImpl.h"
#include "VoxelTools/Impl/VoxelLevelToolsImpl.inl"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelLevelTools::Level(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Falloff,
	float Height,
	bool bAdditive,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	const auto RealHeight = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Height, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelLevelToolsImpl::GetBounds(RealPosition, RealRadius, RealHeight, bAdditive);
	
	GENERATED_TOOL_CALL(Value, FVoxelLevelToolsImpl::Level(Data, RealPosition, RealRadius, Falloff, RealHeight, bAdditive));
}

void UVoxelLevelTools::LevelAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Falloff,
	float Height,
	bool bAdditive,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	const auto RealHeight = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Height, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelLevelToolsImpl::GetBounds(RealPosition, RealRadius, RealHeight, bAdditive);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelLevelToolsImpl::Level(Data, RealPosition, RealRadius, Falloff, RealHeight, bAdditive));
}

void UVoxelLevelTools::Level(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Falloff,
	float Height,
	bool bAdditive,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	const auto RealHeight = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Height, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelLevelToolsImpl::GetBounds(RealPosition, RealRadius, RealHeight, bAdditive);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelLevelToolsImpl::Level(Data, RealPosition, RealRadius, Falloff, RealHeight, bAdditive));
}

void UVoxelLevelTools::LevelAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Falloff,
	float Height,
	bool bAdditive,
	const FOnVoxelToolComplete_WithModifiedValues& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	const auto RealHeight = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Height, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelLevelToolsImpl::GetBounds(RealPosition, RealRadius, RealHeight, bAdditive);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelLevelToolsImpl::Level(Data, RealPosition, RealRadius, Falloff, RealHeight, bAdditive));
}