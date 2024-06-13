// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelNodes/VoxelHeightSplitterNode.h"
#include "Runtime/VoxelNodeType.h"
#include "NodeFunctions/VoxelMathNodeFunctions.h"

UVoxelNode_HeightSplitter::UVoxelNode_HeightSplitter()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}

int32 UVoxelNode_HeightSplitter::GetMinInputPins() const
{
	return 1 + NumSplits * 2;
}

int32 UVoxelNode_HeightSplitter::GetMaxInputPins() const
{
	return GetMinInputPins();
}

int32 UVoxelNode_HeightSplitter::GetOutputPinsCount() const
{
	return NumSplits + 1;
}

FName UVoxelNode_HeightSplitter::GetInputPinName(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "Height";
	}
	PinIndex--;
	if (PinIndex < 2 * NumSplits)
	{
		const int32 SplitIndex = PinIndex / 2;
		if (PinIndex % 2 == 0)
		{
			return *FString::Printf(TEXT("Height %d"), SplitIndex);
		}
		else
		{
			return *FString::Printf(TEXT("Falloff %d"), SplitIndex);
		}
	}
	return "Error";
}

FName UVoxelNode_HeightSplitter::GetOutputPinName(int32 PinIndex) const
{
	return *FString::Printf(TEXT("Layer %d"), PinIndex);
}

FString UVoxelNode_HeightSplitter::GetInputPinDefaultValue(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "0";
	}
	PinIndex--;

	if (PinIndex % 2 == 1)
	{
		return "5";
	}

	PinIndex /= 2;

	const int32 PreviousPinIndex = 2 * (PinIndex - 1) + 1;
	if (InputPins.IsValidIndex(PreviousPinIndex))
	{
		return FString::SanitizeFloat(FCString::Atof(*InputPins[PreviousPinIndex].DefaultValue) + 10.f);
	}

	return FString::SanitizeFloat(PinIndex * 10);
}

