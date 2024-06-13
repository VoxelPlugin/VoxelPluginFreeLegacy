// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelNodes/VoxelPlaceableItemsNodes.h"
#include "Runtime/VoxelNodeType.h"
#include "VoxelContext.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelGraphDataItemConfig.h"
#include "NodeFunctions/VoxelPlaceableItemsNodeFunctions.h"

UVoxelNode_DataItemSample::UVoxelNode_DataItemSample()
{
	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" },
		{ "Smoothness", EC::Float, "The smoothness of the union. The value is a distance: it should be in voxels. If <= 0 Min will be used instead (faster). See SmoothUnion for more info" },
		{ "Default", EC::Float, "The value returned when there are no data item nearby", "10" });
	SetOutputs(EC::Float);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int32 UVoxelNode_DataItemParameters::GetOutputPinsCount() const
{
	return Config ? Config->Parameters.Num() : 0;
}

FName UVoxelNode_DataItemParameters::GetOutputPinName(int32 PinIndex) const
{
	if (!Config || !Config->Parameters.IsValidIndex(PinIndex))
	{
		return STATIC_FNAME("Invalid");
	}
	else
	{
		return Config->Parameters[PinIndex];
	}
}

EVoxelPinCategory UVoxelNode_DataItemParameters::GetOutputPinCategory(int32 PinIndex) const
{
	return EVoxelPinCategory::Float;
}

#if WITH_EDITOR
void UVoxelNode_DataItemParameters::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive && Config)
	{
		const auto OldPreviewValues = MoveTemp(PreviewValues);
		if (Config)
		{
			for (auto& Parameter : Config->Parameters)
			{
				PreviewValues.Add(Parameter, OldPreviewValues.FindRef(Parameter));
			}
		}
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

TArray<v_flt> UVoxelNode_DataItemParameters::GetPreviewValues() const
{
	TArray<v_flt> Result;
	if (Config)
	{
		for (auto& Parameter : Config->Parameters)
		{
			Result.Add(PreviewValues.FindRef(Parameter));
		}
	}
	return Result;
}

