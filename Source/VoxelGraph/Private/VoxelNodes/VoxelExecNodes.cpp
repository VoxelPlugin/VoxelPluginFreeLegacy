// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelExecNodes.h"
#include "VoxelNodes/VoxelNodeColors.h"
#include "VoxelGraphConstants.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelNodeFunctions.h"
#include "VoxelWorldGeneratorInit.h"

#include "EdGraph/EdGraphNode.h"
#include "Async/Async.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"


int32 UVoxelNode_MaterialSetter::GetOutputIndex() const
{
	return FVoxelGraphOutputsIndices::MaterialIndex;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SetColor::UVoxelNode_SetColor()
{
	SetInputs(
		EC::Exec,
		{ "Color", EC::Color, "Color" });
	SetOutputs(EC::Exec);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SetSingleIndex::UVoxelNode_SetSingleIndex()
{
	SetInputs(
		EC::Exec,
		{ "Index", EC::Int, "Index between 0 and 255", "", {0, 255} },
		{ "Data A", EC::Float, "Data sent to the material shader", "", {0, 1} },
		{ "Data B", EC::Float, "Data sent to the material shader", "", {0, 1} },
		{ "Data C", EC::Float, "Data sent to the material shader", "", {0, 1} });
	SetOutputs(EC::Exec);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SetDoubleIndex::UVoxelNode_SetDoubleIndex()
{
	SetInputs(
		EC::Exec,
		{ "Index A", EC::Int, "Index A between 0 and 255", "", {0, 255} },
		{ "Index B", EC::Int, "Index B between 0 and 255", "", {0, 255} },
		{ "Blend", EC::Float, "Blend between 0 and 1", "", {0, 1} },
		{ "Data", EC::Float, "Data sent to material shader", "", {0, 1} });
	SetOutputs(EC::Exec);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SetUVs::UVoxelNode_SetUVs()
{
	SetInputs(
		EC::Exec,
		{ "Channel", EC::Int, "Channel, should be 0 or 1", "", {0, 255 } },
		{ "U", EC::Float, "U coordinate between 0 and 1", "", {0, 1} },
		{ "V", EC::Float, "V coordinate between 0 and 1", "", {0, 1} });
	SetOutputs(EC::Exec);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SetNode::UVoxelNode_SetNode()
{
	SetInputs(EC::Exec, EC::Exec);
	SetOutputs(EC::Exec);
}


FText UVoxelNode_SetNode::GetTitle() const
{
	return FText::FromString("Set " + CachedOutput.Name.ToString());
}

EVoxelPinCategory UVoxelNode_SetNode::GetInputPinCategory(int32 PinIndex) const
{
	return PinIndex == 0
		       ? EVoxelPinCategory::Exec
		       : FVoxelPinCategory::DataPinToPin(CachedOutput.Category);
}

FName UVoxelNode_SetNode::GetInputPinName(int32 PinIndex) const
{
	return PinIndex == 0 ? FName() : CachedOutput.Name;
}

void UVoxelNode_SetNode::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	
#if WITH_EDITOR
	if (!UpdateSetterNode())
	{
		ErrorReporter.AddMessageToNode(this, "invalid output", EVoxelGraphNodeMessageType::FatalError);
	}
#endif
}

int32 UVoxelNode_SetNode::GetOutputIndex() const
{
	return Index;
}

#if WITH_EDITOR
bool UVoxelNode_SetNode::UpdateSetterNode()
{
	if (Graph)
	{
		auto Outputs = Graph->GetOutputs();
		FVoxelGraphOutput NewOutput;
		if (Outputs.Contains(Index) && !FVoxelGraphOutputsUtils::IsVoxelGraphOutputHidden(Index))
		{
			NewOutput = Outputs[Index];
		}
		if (CachedOutput.GUID.IsValid() && NewOutput.GUID != CachedOutput.GUID)
		{
			// Try to find it by GUID and name
			TArray<FVoxelGraphOutput> OutputsArray;
			Outputs.GenerateValueArray(OutputsArray);
			auto* NewOutputPtr = OutputsArray.FindByPredicate([&](auto& Output) {return Output.GUID == CachedOutput.GUID; });
			if (!NewOutputPtr)
			{
				NewOutputPtr = OutputsArray.FindByPredicate([&](auto& Output) {return Output.Name == CachedOutput.Name; });
			}
			if (NewOutputPtr)
			{
				NewOutput = *NewOutputPtr;
				Index = NewOutputPtr->Index;
			}
			else
			{
				return false;
			}
		}
		const bool bDiffCategory = CachedOutput.Category != NewOutput.Category;
		const bool bDiffName = CachedOutput.Name != NewOutput.Name;
		if (GraphNode && (bDiffCategory || bDiffName))
		{
			CachedOutput = NewOutput;
			GraphNode->ReconstructNode();
			if (bDiffCategory)
			{
				Graph->CompileVoxelNodesFromGraphNodes();
			}
		}
	}
	return CachedOutput.GUID.IsValid();
}

void UVoxelNode_SetNode::SetIndex(uint32 NewIndex)
{
	Index = NewIndex;
	UpdateSetterNode();
}

void UVoxelNode_SetNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		UpdateSetterNode();
	}
}

void UVoxelNode_SetNode::PostLoad()
{
	Super::PostLoad();
	UpdateSetterNode();
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FLinearColor UVoxelNode_FunctionSeparator::GetColor() const
{
	return FVoxelNodeColors::ExecNode;
}

EVoxelPinCategory UVoxelNode_FunctionSeparator::GetInputPinCategory(int32 PinIndex) const
{
	return PinIndex == 0
		       ? EVoxelPinCategory::Exec
		       : FVoxelPinCategory::DataPinToPin(ArgTypes[PinIndex - 1].Type);
}

EVoxelPinCategory UVoxelNode_FunctionSeparator::GetOutputPinCategory(int32 PinIndex) const
{
	return PinIndex == 0
		       ? EVoxelPinCategory::Exec
		       : FVoxelPinCategory::DataPinToPin(ArgTypes[PinIndex - 1].Type);
}

FName UVoxelNode_FunctionSeparator::GetInputPinName(int32 PinIndex) const
{
	return PinIndex == 0 ? FName() : FName(*ArgTypes[PinIndex - 1].Name);
}

FName UVoxelNode_FunctionSeparator::GetOutputPinName(int32 PinIndex) const
{
	return FName();
}

int32 UVoxelNode_FunctionSeparator::GetMinInputPins() const
{
	return 1 + ArgTypes.Num();
}

int32 UVoxelNode_FunctionSeparator::GetMaxInputPins() const
{
	return GetMinInputPins();
}

int32 UVoxelNode_FunctionSeparator::GetOutputPinsCount() const
{
	return GetMinInputPins();
}


#if WITH_EDITOR
void UVoxelNode_FunctionSeparator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FLinearColor UVoxelNode_FlowMerge::GetColor() const
{
	return FColor::White;
}

EVoxelPinCategory UVoxelNode_FlowMerge::GetInputPinCategory(int32 PinIndex) const
{
	PinIndex = PinIndex % (Types.Num() + 1);
	return PinIndex == 0 ? EVoxelPinCategory::Exec : FVoxelPinCategory::DataPinToPin(Types[PinIndex - 1].Type);
}

EVoxelPinCategory UVoxelNode_FlowMerge::GetOutputPinCategory(int32 PinIndex) const
{
	return PinIndex == 0 ? EVoxelPinCategory::Exec : FVoxelPinCategory::DataPinToPin(Types[PinIndex - 1].Type);
}

FName UVoxelNode_FlowMerge::GetInputPinName(int32 PinIndex) const
{
	const bool bIsA = PinIndex <= Types.Num();
	PinIndex = PinIndex % (Types.Num() + 1);
	if (PinIndex == 0)
	{
		return bIsA ? FName("Exec A") : FName("Exec B");
	}
	else
	{
		return FName(*(Types[PinIndex - 1].Name + (bIsA ? " A" : " B")));
	}
}

FName UVoxelNode_FlowMerge::GetOutputPinName(int32 PinIndex) const
{
	return PinIndex == 0 ? FName("Exec") : FName(*Types[PinIndex - 1].Name);
}

int32 UVoxelNode_FlowMerge::GetMinInputPins() const
{
	return 2 + 2 * Types.Num();
}

int32 UVoxelNode_FlowMerge::GetMaxInputPins() const
{
	return GetMinInputPins();
}

int32 UVoxelNode_FlowMerge::GetOutputPinsCount() const
{
	return 1 + Types.Num();
}


#if WITH_EDITOR
void UVoxelNode_FlowMerge::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif