// Copyright 2020 Phyronnaz

#include "VoxelTools/Gen/VoxelSphereTools.h"
#include "VoxelTools/Gen/VoxelGeneratedTools.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.h"
#include "VoxelTools/Impl/VoxelSphereToolsImpl.inl"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::SetValueSphere(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Value,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	const auto RealValue = FVoxelValue(Value);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Value, FVoxelSphereToolsImpl::SetValueSphere(Data, RealPosition, RealRadius, RealValue));
}

void UVoxelSphereTools::SetValueSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Value,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	const auto RealValue = FVoxelValue(Value);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSphereToolsImpl::SetValueSphere(Data, RealPosition, RealRadius, RealValue));
}

void UVoxelSphereTools::SetValueSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Value,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	const auto RealValue = FVoxelValue(Value);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSphereToolsImpl::SetValueSphere(Data, RealPosition, RealRadius, RealValue));
}

void UVoxelSphereTools::SetValueSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Value,
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
	const auto RealValue = FVoxelValue(Value);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSphereToolsImpl::SetValueSphere(Data, RealPosition, RealRadius, RealValue));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::RemoveSphere(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Value, FVoxelSphereToolsImpl::RemoveSphere(Data, RealPosition, RealRadius));
}

void UVoxelSphereTools::RemoveSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSphereToolsImpl::RemoveSphere(Data, RealPosition, RealRadius));
}

void UVoxelSphereTools::RemoveSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSphereToolsImpl::RemoveSphere(Data, RealPosition, RealRadius));
}

void UVoxelSphereTools::RemoveSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSphereToolsImpl::RemoveSphere(Data, RealPosition, RealRadius));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::AddSphere(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Value, FVoxelSphereToolsImpl::AddSphere(Data, RealPosition, RealRadius));
}

void UVoxelSphereTools::AddSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSphereToolsImpl::AddSphere(Data, RealPosition, RealRadius));
}

void UVoxelSphereTools::AddSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSphereToolsImpl::AddSphere(Data, RealPosition, RealRadius));
}

void UVoxelSphereTools::AddSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSphereToolsImpl::AddSphere(Data, RealPosition, RealRadius));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::SetMaterialSphere(
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	const FVoxelPaintMaterial& PaintMaterial,
	float Strength,
	EVoxelFalloff FalloffType,
	float Falloff,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Material, FVoxelSphereToolsImpl::SetMaterialSphere(Data, RealPosition, RealRadius, PaintMaterial, Strength, FalloffType, Falloff));
}

void UVoxelSphereTools::SetMaterialSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	const FVoxelPaintMaterial& PaintMaterial,
	float Strength,
	EVoxelFalloff FalloffType,
	float Falloff,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Material, FVoxelSphereToolsImpl::SetMaterialSphere(Data, RealPosition, RealRadius, PaintMaterial, Strength, FalloffType, Falloff));
}

void UVoxelSphereTools::SetMaterialSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	const FVoxelPaintMaterial& PaintMaterial,
	float Strength,
	EVoxelFalloff FalloffType,
	float Falloff,
	TArray<FModifiedVoxelMaterial>* OutModifiedMaterials,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Material, FVoxelSphereToolsImpl::SetMaterialSphere(Data, RealPosition, RealRadius, PaintMaterial, Strength, FalloffType, Falloff));
}

void UVoxelSphereTools::SetMaterialSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	const FVoxelPaintMaterial& PaintMaterial,
	float Strength,
	EVoxelFalloff FalloffType,
	float Falloff,
	const FOnVoxelToolComplete_WithModifiedMaterials& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Material, FVoxelSphereToolsImpl::SetMaterialSphere(Data, RealPosition, RealRadius, PaintMaterial, Strength, FalloffType, Falloff));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::ApplyKernelSphere(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	int32 NumIterations,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Value, FVoxelSphereToolsImpl::ApplyKernelSphere(Data, RealPosition, RealRadius, CenterMultiplier, FirstDegreeNeighborMultiplier, SecondDegreeNeighborMultiplier, ThirdDegreeNeighborMultiplier, NumIterations, FVoxelLambdaUtilities::ConstantStrength));
}

void UVoxelSphereTools::ApplyKernelSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	int32 NumIterations,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSphereToolsImpl::ApplyKernelSphere(Data, RealPosition, RealRadius, CenterMultiplier, FirstDegreeNeighborMultiplier, SecondDegreeNeighborMultiplier, ThirdDegreeNeighborMultiplier, NumIterations, FVoxelLambdaUtilities::ConstantStrength));
}

void UVoxelSphereTools::ApplyKernelSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	int32 NumIterations,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSphereToolsImpl::ApplyKernelSphere(Data, RealPosition, RealRadius, CenterMultiplier, FirstDegreeNeighborMultiplier, SecondDegreeNeighborMultiplier, ThirdDegreeNeighborMultiplier, NumIterations, FVoxelLambdaUtilities::ConstantStrength));
}

void UVoxelSphereTools::ApplyKernelSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	int32 NumIterations,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSphereToolsImpl::ApplyKernelSphere(Data, RealPosition, RealRadius, CenterMultiplier, FirstDegreeNeighborMultiplier, SecondDegreeNeighborMultiplier, ThirdDegreeNeighborMultiplier, NumIterations, FVoxelLambdaUtilities::ConstantStrength));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::ApplyMaterialKernelSphere(
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	int32 NumIterations,
	int32 Mask,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Material, FVoxelSphereToolsImpl::ApplyMaterialKernelSphere(Data, RealPosition, RealRadius, CenterMultiplier, FirstDegreeNeighborMultiplier, SecondDegreeNeighborMultiplier, ThirdDegreeNeighborMultiplier, NumIterations, Mask, FVoxelLambdaUtilities::ConstantStrength));
}

void UVoxelSphereTools::ApplyMaterialKernelSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	int32 NumIterations,
	int32 Mask,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Material, FVoxelSphereToolsImpl::ApplyMaterialKernelSphere(Data, RealPosition, RealRadius, CenterMultiplier, FirstDegreeNeighborMultiplier, SecondDegreeNeighborMultiplier, ThirdDegreeNeighborMultiplier, NumIterations, Mask, FVoxelLambdaUtilities::ConstantStrength));
}

void UVoxelSphereTools::ApplyMaterialKernelSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	int32 NumIterations,
	uint32 Mask,
	TArray<FModifiedVoxelMaterial>* OutModifiedMaterials,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Material, FVoxelSphereToolsImpl::ApplyMaterialKernelSphere(Data, RealPosition, RealRadius, CenterMultiplier, FirstDegreeNeighborMultiplier, SecondDegreeNeighborMultiplier, ThirdDegreeNeighborMultiplier, NumIterations, Mask, FVoxelLambdaUtilities::ConstantStrength));
}

void UVoxelSphereTools::ApplyMaterialKernelSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float CenterMultiplier,
	float FirstDegreeNeighborMultiplier,
	float SecondDegreeNeighborMultiplier,
	float ThirdDegreeNeighborMultiplier,
	int32 NumIterations,
	uint32 Mask,
	const FOnVoxelToolComplete_WithModifiedMaterials& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Material, FVoxelSphereToolsImpl::ApplyMaterialKernelSphere(Data, RealPosition, RealRadius, CenterMultiplier, FirstDegreeNeighborMultiplier, SecondDegreeNeighborMultiplier, ThirdDegreeNeighborMultiplier, NumIterations, Mask, FVoxelLambdaUtilities::ConstantStrength));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::SmoothSphere(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Strength,
	int32 NumIterations,
	EVoxelFalloff FalloffType,
	float Falloff,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Value, FVoxelSphereToolsImpl::SmoothSphere(Data, RealPosition, RealRadius, Strength, NumIterations, FalloffType, Falloff));
}

void UVoxelSphereTools::SmoothSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Strength,
	int32 NumIterations,
	EVoxelFalloff FalloffType,
	float Falloff,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSphereToolsImpl::SmoothSphere(Data, RealPosition, RealRadius, Strength, NumIterations, FalloffType, Falloff));
}

void UVoxelSphereTools::SmoothSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Strength,
	int32 NumIterations,
	EVoxelFalloff FalloffType,
	float Falloff,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSphereToolsImpl::SmoothSphere(Data, RealPosition, RealRadius, Strength, NumIterations, FalloffType, Falloff));
}

void UVoxelSphereTools::SmoothSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Strength,
	int32 NumIterations,
	EVoxelFalloff FalloffType,
	float Falloff,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSphereToolsImpl::SmoothSphere(Data, RealPosition, RealRadius, Strength, NumIterations, FalloffType, Falloff));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::SmoothMaterialSphere(
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Strength,
	int32 NumIterations,
	int32 Mask,
	EVoxelFalloff FalloffType,
	float Falloff,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Material, FVoxelSphereToolsImpl::SmoothMaterialSphere(Data, RealPosition, RealRadius, Strength, NumIterations, Mask, FalloffType, Falloff));
}

void UVoxelSphereTools::SmoothMaterialSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelMaterial>& ModifiedMaterials,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Strength,
	int32 NumIterations,
	int32 Mask,
	EVoxelFalloff FalloffType,
	float Falloff,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Material, FVoxelSphereToolsImpl::SmoothMaterialSphere(Data, RealPosition, RealRadius, Strength, NumIterations, Mask, FalloffType, Falloff));
}

void UVoxelSphereTools::SmoothMaterialSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Strength,
	int32 NumIterations,
	uint32 Mask,
	EVoxelFalloff FalloffType,
	float Falloff,
	TArray<FModifiedVoxelMaterial>* OutModifiedMaterials,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Material, FVoxelSphereToolsImpl::SmoothMaterialSphere(Data, RealPosition, RealRadius, Strength, NumIterations, Mask, FalloffType, Falloff));
}

void UVoxelSphereTools::SmoothMaterialSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	float Strength,
	int32 NumIterations,
	uint32 Mask,
	EVoxelFalloff FalloffType,
	float Falloff,
	const FOnVoxelToolComplete_WithModifiedMaterials& Callback,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bRecordModifiedMaterials,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_ASYNC_CPP(Material);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Material, FVoxelSphereToolsImpl::SmoothMaterialSphere(Data, RealPosition, RealRadius, Strength, NumIterations, Mask, FalloffType, Falloff));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::TrimSphere(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	const FVector& Normal,
	float Radius,
	float Falloff,
	bool bAdditive,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Value, FVoxelSphereToolsImpl::TrimSphere(Data, RealPosition, Normal, RealRadius, Falloff, bAdditive));
}

void UVoxelSphereTools::TrimSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	const FVector& Normal,
	float Radius,
	float Falloff,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSphereToolsImpl::TrimSphere(Data, RealPosition, Normal, RealRadius, Falloff, bAdditive));
}

void UVoxelSphereTools::TrimSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	const FVector& Normal,
	float Radius,
	float Falloff,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSphereToolsImpl::TrimSphere(Data, RealPosition, Normal, RealRadius, Falloff, bAdditive));
}

void UVoxelSphereTools::TrimSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	const FVector& Normal,
	float Radius,
	float Falloff,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSphereToolsImpl::TrimSphere(Data, RealPosition, Normal, RealRadius, Falloff, bAdditive));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::RevertSphere(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	int32 HistoryPosition,
	bool bRevertValues,
	bool bRevertMaterials,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Value, FVoxelSphereToolsImpl::RevertSphere(Data, RealPosition, RealRadius, HistoryPosition, bRevertValues, bRevertMaterials));
}

void UVoxelSphereTools::RevertSphereAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	int32 HistoryPosition,
	bool bRevertValues,
	bool bRevertMaterials,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSphereToolsImpl::RevertSphere(Data, RealPosition, RealRadius, HistoryPosition, bRevertValues, bRevertMaterials));
}

void UVoxelSphereTools::RevertSphere(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	int32 HistoryPosition,
	bool bRevertValues,
	bool bRevertMaterials,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSphereToolsImpl::RevertSphere(Data, RealPosition, RealRadius, HistoryPosition, bRevertValues, bRevertMaterials));
}

void UVoxelSphereTools::RevertSphereAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	int32 HistoryPosition,
	bool bRevertValues,
	bool bRevertMaterials,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSphereToolsImpl::RevertSphere(Data, RealPosition, RealRadius, HistoryPosition, bRevertValues, bRevertMaterials));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void UVoxelSphereTools::RevertSphereToGenerator(
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	bool bRevertValues,
	bool bRevertMaterials,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL(Value, FVoxelSphereToolsImpl::RevertSphereToGenerator(Data, RealPosition, RealRadius, bRevertValues, bRevertMaterials));
}

void UVoxelSphereTools::RevertSphereToGeneratorAsync(
	UObject* WorldContextObject,
	FLatentActionInfo LatentInfo,
	TArray<FModifiedVoxelValue>& ModifiedValues,
	FVoxelIntBox& EditedBounds,
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	bool bRevertValues,
	bool bRevertMaterials,
	bool bMultiThreaded,
	bool bRecordModifiedValues,
	bool bConvertToVoxelSpace,
	bool bUpdateRender,
	bool bHideLatentWarnings)
{
	GENERATED_TOOL_FUNCTION_ASYNC(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC(Value, FVoxelSphereToolsImpl::RevertSphereToGenerator(Data, RealPosition, RealRadius, bRevertValues, bRevertMaterials));
}

void UVoxelSphereTools::RevertSphereToGenerator(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	bool bRevertValues,
	bool bRevertMaterials,
	TArray<FModifiedVoxelValue>* OutModifiedValues,
	FVoxelIntBox* OutEditedBounds,
	bool bMultiThreaded,
	bool bConvertToVoxelSpace,
	bool bUpdateRender)
{
	GENERATED_TOOL_FUNCTION_CPP(Value);
	
	const auto RealPosition = FVoxelToolHelpers::GetRealPosition(VoxelWorld, Position, bConvertToVoxelSpace);
	const auto RealRadius = FVoxelToolHelpers::GetRealDistance(VoxelWorld, Radius, bConvertToVoxelSpace);
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_CPP(Value, FVoxelSphereToolsImpl::RevertSphereToGenerator(Data, RealPosition, RealRadius, bRevertValues, bRevertMaterials));
}

void UVoxelSphereTools::RevertSphereToGeneratorAsync(
	AVoxelWorld* VoxelWorld,
	const FVector& Position,
	float Radius,
	bool bRevertValues,
	bool bRevertMaterials,
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
	
	const FVoxelIntBox Bounds = FVoxelSphereToolsImpl::GetBounds(RealPosition, RealRadius);
	
	GENERATED_TOOL_CALL_ASYNC_CPP(Value, FVoxelSphereToolsImpl::RevertSphereToGenerator(Data, RealPosition, RealRadius, bRevertValues, bRevertMaterials));
}