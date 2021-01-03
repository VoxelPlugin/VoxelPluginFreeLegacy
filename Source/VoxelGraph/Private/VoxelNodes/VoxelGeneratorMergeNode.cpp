// Copyright 2021 Phyronnaz

#include "VoxelNodes/VoxelGeneratorMergeNode.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelGenerators/VoxelGeneratorInit.h"
#include "VoxelGenerators/VoxelGeneratorInstance.h"
#include "NodeFunctions/VoxelNodeFunctions.h"

constexpr int32 NumDefaultInputPins_WGMN = 3 + 2 * 4;


UVoxelNode_GeneratorMerge::UVoxelNode_GeneratorMerge()
{
	SetInputs({
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" },
		{ "Index 0", EC::Int, "First generator index" },
		{ "Alpha 0", EC::Float, "First generator alpha" },
		{ "Index 1", EC::Int, "Second generator index" },
		{ "Alpha 1", EC::Float, "Second generator alpha" },
		{ "Index 2", EC::Int, "Third generator index" },
		{ "Alpha 2", EC::Float, "Third generator alpha" },
		{ "Index 3", EC::Int, "Fourth generator index" },
		{ "Alpha 3", EC::Float, "Fourth generator alpha" } });
	check(UVoxelNodeHelper::GetMinInputPins() == NumDefaultInputPins_WGMN);
}

FText UVoxelNode_GeneratorMerge::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Generator Merge: {0}"), Super::GetTitle());
}

int32 UVoxelNode_GeneratorMerge::GetOutputPinsCount() const
{
	return 2 + Outputs.Num() + 1;
}

FName UVoxelNode_GeneratorMerge::GetOutputPinName(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "Value";
	}
	if (PinIndex == 1)
	{
		return "Material";
	}
	if (PinIndex == GetOutputPinsCount() - 1)
	{
		return "Num Queried Generators";
	}
	PinIndex -= 2;
	if (Outputs.IsValidIndex(PinIndex))
	{
		return Outputs[PinIndex];
	}
	return "Error";
}

EVoxelPinCategory UVoxelNode_GeneratorMerge::GetOutputPinCategory(int32 PinIndex) const
{
	if (PinIndex == 1)
	{
		return EC::Material;
	}
	else if (PinIndex == GetOutputPinsCount() - 1)
	{
		return EC::Int;
	}
	else
	{
		return EC::Float;
	}
}

void UVoxelNode_GeneratorMerge::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	
	for (auto& Generator : Generators)
	{
		if (!Generator.IsValid())
		{
			ErrorReporter.AddMessageToNode(this, "invalid generator", EVoxelGraphNodeMessageType::Error);
		}
	}
}