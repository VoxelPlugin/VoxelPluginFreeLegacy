// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelNodeHelper.h"
#include "VoxelNodes/VoxelNodeColors.h"
#include "VoxelAxisDependencies.h"

EVoxelPinCategory UVoxelNodeHelper::GetInputPinCategory(int32 PinIndex) const
{
	return Pins.GetInputPin(PinIndex).Category;
}

EVoxelPinCategory UVoxelNodeHelper::GetOutputPinCategory(int32 PinIndex) const
{
	return Pins.GetOutputPin(PinIndex).Category;
}

FName UVoxelNodeHelper::GetInputPinName(int32 PinIndex) const
{
	return Pins.GetInputPin(PinIndex).Name;
}

FName UVoxelNodeHelper::GetOutputPinName(int32 PinIndex) const
{
	return Pins.GetOutputPin(PinIndex).Name;
}

FString UVoxelNodeHelper::GetInputPinToolTip(int32 PinIndex) const
{
	return Pins.GetInputPin(PinIndex).ToolTip;
}

FString UVoxelNodeHelper::GetOutputPinToolTip(int32 PinIndex) const
{
	return Pins.GetOutputPin(PinIndex).ToolTip;
}

int32 UVoxelNodeHelper::GetMinInputPins() const
{
	return bCustomInputsCount ? CustomMin : Pins.InputPins.Num();
}

int32 UVoxelNodeHelper::GetMaxInputPins() const
{
	return bCustomInputsCount ? CustomMax : Pins.InputPins.Num();
}

int32 UVoxelNodeHelper::GetInputPinsIncrement() const
{
	return Increment;
}

int32 UVoxelNodeHelper::GetOutputPinsCount() const
{
	return Pins.OutputPins.Num();
}

FLinearColor UVoxelNodeHelper::GetColor() const
{
	return Color;
}

FVoxelPinDefaultValueBounds UVoxelNodeHelper::GetInputPinDefaultValueBounds(int32 PinIndex) const
{
	return Pins.GetInputPin(PinIndex).DefaultValueBounds;
}

FString UVoxelNodeHelper::GetInputPinDefaultValue(int32 PinIndex) const
{
	return Pins.GetInputPin(PinIndex).DefaultValue;
}

UVoxelSetterNode::UVoxelSetterNode()
{
	SetColor(FVoxelNodeColors::ExecNode);
}


uint8 UVoxelNodeWithContext::GetNodeDependencies() const
{
	return EVoxelAxisDependenciesFlags::X;
}