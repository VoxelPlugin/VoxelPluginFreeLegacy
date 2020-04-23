// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelGraphMacro.h"
#include "VoxelNodes/VoxelNodeColors.h"
#include "VoxelNodes/VoxelLocalVariables.h"
#include "Compilation/VoxelCompilationEnums.h"
#include "VoxelGraphErrorReporter.h"

FLinearColor UVoxelGraphMacroInputOutputNode::GetColor() const
{
	return FVoxelNodeColors::ExecNode;
}

bool UVoxelGraphMacroInputOutputNode::CanUserDeleteNode() const
{
	return false;
}

bool UVoxelGraphMacroInputOutputNode::CanDuplicateNode() const
{
	return false;
}

int32 UVoxelGraphMacroInputOutputNode::GetMaxInputPins() const
{
	return Pins.Num();
}

int32 UVoxelGraphMacroInputOutputNode::GetMinInputPins() const
{
	return Pins.Num();
}

EVoxelPinCategory UVoxelGraphMacroInputOutputNode::GetInputPinCategory(int32 PinIndex) const
{
	return Pins[PinIndex].Category;
}

int32 UVoxelGraphMacroInputOutputNode::GetOutputPinsCount() const
{
	return Pins.Num();
}

EVoxelPinCategory UVoxelGraphMacroInputOutputNode::GetOutputPinCategory(int32 PinIndex) const
{
	return Pins[PinIndex].Category;
}


#if WITH_EDITOR
void UVoxelGraphMacroInputOutputNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && Graph && GraphNode)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}

	if (IsA<UVoxelGraphMacroOutputNode>())
	{
		for (auto& Pin : Pins)
		{
			Pin.bCustomDefaultValue = false;
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UVoxelGraphMacroInputOutputNode::PostLoad()
{
	Super::PostLoad();

	if (GraphNode && InputPins.Num() != OutputPins.Num())
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif // WITH_EDITOR

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline FVoxelGraphMacroPin GetPin(UVoxelGraphMacro* Macro, EVoxelPinDirection Direction, int32 PinIndex)
{
	if (Macro)
	{
		if (auto* Node = Direction == EVoxelPinDirection::Input ?
			static_cast<UVoxelGraphMacroInputOutputNode*>(Macro->InputNode) :
			static_cast<UVoxelGraphMacroInputOutputNode*>(Macro->OutputNode))
		{
			auto& Pins = Node->Pins;
			if (Pins.IsValidIndex(PinIndex))
			{
				return Pins[PinIndex];
			}
		}
	}
	return FVoxelGraphMacroPin();
}

FText UVoxelGraphMacro::GetMacroName() const
{
	if (CustomName.IsEmpty())
	{
		return FText::FromString(FName::NameToDisplayString(GetName(), false));
	}
	else
	{
		return FText::FromString(CustomName);
	}
}

FText UVoxelGraphMacro::GetMacroCategory() const
{
	return CustomCategory.IsEmpty() ? VOXEL_LOCTEXT("Macro nodes") : FText::FromString(CustomCategory);
}


FText UVoxelGraphMacroNode::GetTitle() const
{
	return Macro ? Macro->GetMacroName() : Super::GetTitle();
}

FText UVoxelGraphMacroNode::GetTooltip() const
{
	return Macro ? FText::FromString(Macro->Tooltip) : Super::GetTooltip();
}

int32 UVoxelGraphMacroNode::GetMaxInputPins() const
{
	return Macro && Macro->InputNode ? Macro->InputNode->Pins.Num() : 0;
}

int32 UVoxelGraphMacroNode::GetMinInputPins() const
{
	return Macro && Macro->InputNode ? Macro->InputNode->Pins.Num() : 0;
}

int32 UVoxelGraphMacroNode::GetOutputPinsCount() const
{
	return Macro && Macro->OutputNode ? Macro->OutputNode->Pins.Num() : 0;
}

FName UVoxelGraphMacroNode::GetInputPinName(int32 PinIndex) const
{
	return *GetPin(Macro, EVoxelPinDirection::Input, PinIndex).Name;
}

FName UVoxelGraphMacroNode::GetOutputPinName(int32 PinIndex) const
{
	return *GetPin(Macro, EVoxelPinDirection::Output, PinIndex).Name;
}

FString UVoxelGraphMacroNode::GetInputPinToolTip(int32 PinIndex) const
{
	return *GetPin(Macro, EVoxelPinDirection::Input, PinIndex).ToolTip;
}

FString UVoxelGraphMacroNode::GetOutputPinToolTip(int32 PinIndex) const
{
	return *GetPin(Macro, EVoxelPinDirection::Output, PinIndex).ToolTip;
}

EVoxelPinCategory UVoxelGraphMacroNode::GetInputPinCategory(int32 PinIndex) const
{
	return GetPin(Macro, EVoxelPinDirection::Input, PinIndex).Category;
}

EVoxelPinCategory UVoxelGraphMacroNode::GetOutputPinCategory(int32 PinIndex) const
{
	return GetPin(Macro, EVoxelPinDirection::Output, PinIndex).Category;
}

FString UVoxelGraphMacroNode::GetInputPinDefaultValue(int32 PinIndex) const
{
	return GetPin(Macro, EVoxelPinDirection::Input, PinIndex).DefaultValue;
}

inline bool ArePinsArraysSameNameAndCategory(const TArray<FVoxelPin>& A, const TArray<FVoxelPin>& B)
{
	if (A.Num() != B.Num())
	{
		return false;
	}
	for (int32 Index = 0; Index < A.Num(); Index++)
	{
		if (A[Index].PinCategory != B[Index].PinCategory)
		{
			return false;
		}
	}
	return true;
}

void UVoxelGraphMacroNode::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	
	if (!Macro)
	{
		ErrorReporter.AddMessageToNode(this, "invalid macro ref", EVoxelGraphNodeMessageType::FatalError);
	}
	else if (!Macro->InputNode || !Macro->OutputNode)
	{
		ErrorReporter.AddMessageToNode(this, "corrupted macro", EVoxelGraphNodeMessageType::FatalError);
	}
	else if (!ArePinsArraysSameNameAndCategory(InputPins, Macro->InputNode->OutputPins) || 
		     !ArePinsArraysSameNameAndCategory(OutputPins, Macro->OutputNode->InputPins))
	{
		ErrorReporter.AddError("Outdated macro: please right click it and press Reconstruct Node");
		ErrorReporter.AddMessageToNode(this, "outdated macro", EVoxelGraphNodeMessageType::FatalError);
	}
}

#if WITH_EDITOR
void UVoxelGraphMacroNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{	
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelGraphMacroNode, Macro) && Graph && GraphNode)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif // WITH_EDITOR