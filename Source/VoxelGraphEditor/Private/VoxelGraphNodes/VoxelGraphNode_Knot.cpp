// Copyright 2020 Phyronnaz

#include "VoxelGraphNode_Knot.h"
#include "EdGraph/EdGraphPin.h"
#include "VoxelPinCategory.h"

const static FName Wildcard(FVoxelPinCategory::GetName(EVoxelPinCategory::Wildcard));

void UVoxelGraphNode_Knot::CreateInputPins()
{
	UEdGraphPin* InputPin = CreatePin(EGPD_Input, Wildcard, "InputPin");
	InputPin->bDefaultValueIsIgnored = true;
}

void UVoxelGraphNode_Knot::CreateOutputPins()
{
	CreatePin(EGPD_Output, Wildcard, "OutputPin");
}

void UVoxelGraphNode_Knot::ReconstructNode()
{
	Super::ReconstructNode();
	PropagatePinType();
}

FText UVoxelGraphNode_Knot::GetTooltipText() const
{
	return VOXEL_LOCTEXT("Reroute Node (reroutes wires)");
}

FText UVoxelGraphNode_Knot::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (TitleType == ENodeTitleType::EditableTitle)
	{
		return FText::FromString(NodeComment);
	}
	else if (TitleType == ENodeTitleType::MenuTitle)
	{
		return VOXEL_LOCTEXT("Add Reroute Node...");
	}
	else
	{
		return VOXEL_LOCTEXT("Reroute Node");
	}
}

bool UVoxelGraphNode_Knot::CanSplitPin(const UEdGraphPin* Pin) const
{
	return false;
}

void UVoxelGraphNode_Knot::PropagatePinType()
{
	UEdGraphPin* InputPin  = GetInputPin();
	UEdGraphPin* OutputPin = GetOutputPin();

	for (UEdGraphPin* Inputs : InputPin->LinkedTo)
	{
		if (Inputs->PinType.PinCategory != Wildcard)
		{
			PropagatePinTypeFromInput();
			return;
		}
	}

	for (UEdGraphPin* Outputs : OutputPin->LinkedTo)
	{
		if (Outputs->PinType.PinCategory != Wildcard)
		{
			PropagatePinTypeFromOutput();
			return;
		}
	}

	// if all inputs/outputs are wildcards, still favor the inputs first (propagate array/reference/etc. state)
	if (InputPin->LinkedTo.Num() > 0)
	{
		// If we can't mirror from output type, we should at least get the type information from the input connection chain
		PropagatePinTypeFromInput();
	}
	else if (OutputPin->LinkedTo.Num() > 0)
	{
		// Try to mirror from output first to make sure we get appropriate member references
		PropagatePinTypeFromOutput();
	}
	else
	{
		// Revert to wildcard
		InputPin->BreakAllPinLinks();
		InputPin->PinType.ResetToDefaults();
		InputPin->PinType.PinCategory = Wildcard;

		OutputPin->BreakAllPinLinks();
		OutputPin->PinType.ResetToDefaults();
		OutputPin->PinType.PinCategory = Wildcard;
	}
}

TArray<UEdGraphPin*> UVoxelGraphNode_Knot::GetAllInputPins()
{
	TArray<UEdGraphPin*> KnotRecursiveInputPins;
	{
		TArray<UVoxelGraphNode_Knot*> KnotsToProcess;
		KnotsToProcess.Add(this);

		while (KnotsToProcess.Num() > 0)
		{
			auto CurrentKnot = KnotsToProcess.Pop();
			auto InputPin = CurrentKnot->GetInputPin();
			for (auto& Pin : InputPin->LinkedTo)
			{
				auto Knot = Cast<UVoxelGraphNode_Knot>(Pin->GetOwningNode());
				if (Knot)
				{
					KnotsToProcess.Add(Knot);
				}
				else
				{
					KnotRecursiveInputPins.Add(Pin);
				}
			}
		}
	}
	return KnotRecursiveInputPins;
}

TArray<UEdGraphPin*> UVoxelGraphNode_Knot::GetAllOutputPins()
{
	TArray<UEdGraphPin*> KnotRecursiveOutputPins;
	{
		TArray<UVoxelGraphNode_Knot*> KnotsToProcess;
		KnotsToProcess.Add(this);

		while (KnotsToProcess.Num() > 0)
		{
			auto CurrentKnot = KnotsToProcess.Pop();
			auto OutputPin = CurrentKnot->GetOutputPin();
			for (auto& Pin : OutputPin->LinkedTo)
			{
				auto Knot = Cast<UVoxelGraphNode_Knot>(Pin->GetOwningNode());
				if (Knot)
				{
					KnotsToProcess.Add(Knot);
				}
				else
				{
					KnotRecursiveOutputPins.Add(Pin);
				}
			}
		}
	}
	return KnotRecursiveOutputPins;
}

void UVoxelGraphNode_Knot::PropagatePinTypeFromInput()
{
	if (bRecursionGuard)
	{
		return;
	}
	// Set the type of the pin based on input connections.
	// We have to move up the chain of linked reroute nodes until we reach a node
	// with type information before percolating that information down.
	UEdGraphPin* MyInputPin = GetInputPin();
	UEdGraphPin* MyOutputPin = GetOutputPin();

	TGuardValue<bool> RecursionGuard(bRecursionGuard, true);

	for (UEdGraphPin* InPin : MyInputPin->LinkedTo)
	{
		if (UVoxelGraphNode_Knot* KnotNode = Cast<UVoxelGraphNode_Knot>(InPin->GetOwningNode()))
		{
			KnotNode->PropagatePinTypeFromInput();
		}
	}

	UEdGraphPin* TypeSource = MyInputPin->LinkedTo.Num() ? MyInputPin->LinkedTo[0] : nullptr;
	if (TypeSource)
	{
		MyInputPin->PinType = TypeSource->PinType;
		MyOutputPin->PinType = TypeSource->PinType;
	}
}

void UVoxelGraphNode_Knot::PropagatePinTypeFromOutput()
{
	if (bRecursionGuard)
	{
		return;
	}
	// Set the type of the pin based on the output connection, and then percolate
	// that type information up until we no longer reach another Reroute node
	UEdGraphPin* InputPin = GetInputPin();
	UEdGraphPin* OutputPin = GetOutputPin();

	TGuardValue<bool> RecursionGuard(bRecursionGuard, true);

	for (UEdGraphPin* InPin : OutputPin->LinkedTo)
	{
		if (UVoxelGraphNode_Knot* KnotNode = Cast<UVoxelGraphNode_Knot>(InPin->GetOwningNode()))
		{
			KnotNode->PropagatePinTypeFromOutput();
		}
	}

	UEdGraphPin* TypeSource = OutputPin->LinkedTo.Num() ? OutputPin->LinkedTo[0] : nullptr;
	if (TypeSource)
	{
		InputPin->PinType = TypeSource->PinType;
		OutputPin->PinType = TypeSource->PinType;
	}
}

bool UVoxelGraphNode_Knot::ShouldOverridePinNames() const
{
	return true;
}

FText UVoxelGraphNode_Knot::GetPinNameOverride(const UEdGraphPin& Pin) const
{
	// Keep the pin size tiny
	return FText::GetEmpty();
}

void UVoxelGraphNode_Knot::OnRenameNode(const FString& NewName)
{
	NodeComment = NewName;
}

UEdGraphPin* UVoxelGraphNode_Knot::GetPassThroughPin(const UEdGraphPin* FromPin) const
{
	if (FromPin && Pins.Contains(FromPin))
	{
		return FromPin == Pins[0] ? Pins[1] : Pins[0];
	}

	return nullptr;
}