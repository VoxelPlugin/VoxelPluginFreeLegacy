// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelGraphAssetNodes.h"
#include "VoxelNodes/VoxelWorldGeneratorSamplerNodes.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelWorldGenerator.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelNodeFunctions.h"
#include "VoxelTools/VoxelHardnessHandler.h"
#include "VoxelGraphGenerator.h"

EVoxelPinCategory UVoxelGraphAssetNode::GetInputPinCategory(int32 PinIndex) const
{
	const int32 NumDefaultInputPins = Super::GetMinInputPins();
	if (PinIndex < NumDefaultInputPins)
	{
		return Super::GetInputPinCategory(PinIndex);
	}
	PinIndex -= NumDefaultInputPins;
	if (CustomData.IsValidIndex(PinIndex))
	{
		return EC::Float;
	}
	return EC::Float;
}

FName UVoxelGraphAssetNode::GetInputPinName(int32 PinIndex) const
{
	const int32 NumDefaultInputPins = Super::GetMinInputPins();
	if (PinIndex < NumDefaultInputPins)
	{
		return Super::GetInputPinName(PinIndex);
	}
	PinIndex -= NumDefaultInputPins;
	if (CustomData.IsValidIndex(PinIndex))
	{
		return CustomData[PinIndex];
	}
	return "ERROR";
}

int32 UVoxelGraphAssetNode::GetMinInputPins() const
{
	return Super::GetMinInputPins() + CustomData.Num();
}

int32 UVoxelGraphAssetNode::GetMaxInputPins() const
{
	return GetMinInputPins();
}

#if WITH_EDITOR
void UVoxelGraphAssetNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (Graph && GraphNode && PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_EditGetValue::UVoxelNode_EditGetValue()
{
	SetInputs(
		{ "X", EC::Float, "X in global space. Use Global X" },
		{ "Y", EC::Float, "Y in global space. Use Global Y" },
		{ "Z", EC::Float, "Z in global space. Use Global Z" }
	);
	SetOutputs(
		EC::Float
	);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_EditGetMaterial::UVoxelNode_EditGetMaterial()
{
	SetInputs(
		{ "X", EC::Float, "X in global space. Use Global X" },
		{ "Y", EC::Float, "Y in global space. Use Global Y" },
		{ "Z", EC::Float, "Z in global space. Use Global Z" }
	);
	SetOutputs(
		EC::Material
	);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_EditGetCustomOutput::UVoxelNode_EditGetCustomOutput()
{
	SetInputs(
		{ "X", EC::Float, "X in global space. Use Global X" },
		{ "Y", EC::Float, "Y in global space. Use Global Y" },
		{ "Z", EC::Float, "Z in global space. Use Global Z" }
	);
	SetOutputs(
		EC::Float
	);
}

FText UVoxelNode_EditGetCustomOutput::GetTitle() const
{
	return FText::FromString("Get Previous Generator Custom Output: " + OutputName.ToString());
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_EditGetHardness::UVoxelNode_EditGetHardness()
{
	SetInputs(
		EC::Material
	);
	SetOutputs(
		EC::Float
	);
}

