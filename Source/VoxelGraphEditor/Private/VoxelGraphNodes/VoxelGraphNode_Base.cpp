// Copyright 2020 Phyronnaz

#include "VoxelGraphNode_Base.h"
#include "VoxelGraphSchema.h"
#include "VoxelPinCategory.h"
#include "EdGraph/EdGraphSchema.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraph/EdGraph.h"

TArray<UEdGraphPin*> UVoxelGraphNode_Base::GetOutputPins() const
{
	return Pins.FilterByPredicate([&](const UEdGraphPin* Pin) { return Pin->Direction == EGPD_Output; });
}

TArray<UEdGraphPin*> UVoxelGraphNode_Base::GetInputPins() const
{
	return Pins.FilterByPredicate([&](const UEdGraphPin* Pin) { return Pin->Direction == EGPD_Input; });
}

UEdGraphPin* UVoxelGraphNode_Base::GetInputPin(int32 InputIndex)
{
	check(InputIndex >= 0 && InputIndex < GetInputCount());

	for (int32 PinIndex = 0, FoundInputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Input)
		{
			if (InputIndex == FoundInputs)
			{
				return Pins[PinIndex];
			}
			else
			{
				FoundInputs++;
			}
		}
	}

	return nullptr;
}

UEdGraphPin* UVoxelGraphNode_Base::GetOutputPin(int32 OutputIndex)
{
	check(OutputIndex >= 0 && OutputIndex < GetOutputCount());

	for (int32 PinIndex = 0, FoundOutputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Output)
		{
			if (OutputIndex == FoundOutputs)
			{
				return Pins[PinIndex];
			}
			else
			{
				FoundOutputs++;
			}
		}
	}

	return nullptr;
}

int32 UVoxelGraphNode_Base::GetInputPinIndex(const UEdGraphPin* Pin) const
{
	for (int32 PinIndex = 0, FoundInputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Input)
		{
			if (Pins[PinIndex] == Pin)
			{
				return FoundInputs;
			}
			else
			{
				FoundInputs++;
			}
		}
	}

	return -1;
}

int32 UVoxelGraphNode_Base::GetOutputPinIndex(const UEdGraphPin* Pin) const
{
	for (int32 PinIndex = 0, FoundOutputs = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Output)
		{
			if (Pins[PinIndex] == Pin)
			{
				return FoundOutputs;
			}
			else
			{
				FoundOutputs++;
			}
		}
	}

	return -1;
}

int32 UVoxelGraphNode_Base::GetInputCount() const
{
	int32 InputCount = 0;

	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Input)
		{
			InputCount++;
		}
	}

	return InputCount;
}

int32 UVoxelGraphNode_Base::GetOutputCount() const
{
	int32 OutputCount = 0;

	for (int32 PinIndex = 0; PinIndex < Pins.Num(); PinIndex++)
	{
		if (Pins[PinIndex]->Direction == EGPD_Output)
		{
			OutputCount++;
		}
	}

	return OutputCount;
}

void UVoxelGraphNode_Base::InsertNewNode(UEdGraphPin* FromPin, UEdGraphPin* NewLinkPin, TSet<UEdGraphNode*>& OutNodeList)
{
	const UVoxelGraphSchema* Schema = CastChecked<UVoxelGraphSchema>(GetSchema());

	// The pin we are creating from already has a connection that needs to be broken. We want to "insert" the new node in between, so that the output of the new node is hooked up too
	UEdGraphPin* OldLinkedPin = FromPin->LinkedTo[0];
	check(OldLinkedPin);

	FromPin->BreakAllPinLinks();

	// Hook up the old linked pin to the first valid output pin on the new node
	for (int32 OutpinPinIdx = 0; OutpinPinIdx < Pins.Num(); OutpinPinIdx++)
	{
		UEdGraphPin* OutputExecPin = Pins[OutpinPinIdx];
		check(OutputExecPin);
		if (ECanCreateConnectionResponse::CONNECT_RESPONSE_MAKE == Schema->CanCreateConnection(OldLinkedPin, OutputExecPin).Response)
		{
			if (Schema->TryCreateConnection(OldLinkedPin, OutputExecPin))
			{
				OutNodeList.Add(OldLinkedPin->GetOwningNode());
				OutNodeList.Add(this);
			}
			break;
		}
	}

	if (Schema->TryCreateConnection(FromPin, NewLinkPin))
	{
		OutNodeList.Add(FromPin->GetOwningNode());
		OutNodeList.Add(this);
	}
}

void UVoxelGraphNode_Base::AllocateDefaultPins()
{
	check(Pins.Num() == 0);

	CreateInputPins();
	CreateOutputPins();
}

inline bool MovePin(UEdGraphPin* OldPin, UEdGraphPin* NewPin)
{
	const auto OldType = FVoxelPinCategory::FromString(OldPin->PinType.PinCategory);
	const auto NewType = FVoxelPinCategory::FromString(NewPin->PinType.PinCategory);
	if (OldType == NewType ||
		OldType == EVoxelPinCategory::Wildcard ||
		NewType == EVoxelPinCategory::Wildcard)
	{
		NewPin->MovePersistentDataFromOldPin(*OldPin);
		return true;
	}
	else
	{
		return false;
	}
}

inline void MovePins(const TArray<UEdGraphPin*>& OldPins, const TArray<UEdGraphPin*>& NewPins)
{
	TSet<UEdGraphPin*> MovedOldPins;
	TSet<UEdGraphPin*> MovedNewPins;

	// Tricky case: renaming a macro node pin
	
	// First try the PinId
	for (auto* OldPin : OldPins)
	{
		UEdGraphPin* const * NewPinPtr = nullptr;
		if (!NewPinPtr)
		{
			NewPinPtr = NewPins.FindByPredicate([&](auto* NewPin) { return NewPin->PinId == OldPin->PinId; });
		}
		if (NewPinPtr)
		{
			auto* NewPin = *NewPinPtr;
			if (!MovedNewPins.Contains(NewPin) && MovePin(OldPin, NewPin))
			{
				MovedOldPins.Add(OldPin);
				MovedNewPins.Add(NewPin);
			}
		}
	}

	// Else use the index
	for (int32 Index = 0; Index < OldPins.Num(); Index++)
	{
		auto* OldPin = OldPins[Index];
		if (!MovedOldPins.Contains(OldPin) && NewPins.IsValidIndex(Index))
		{
			auto* NewPin = NewPins[Index];
			if (!MovedNewPins.Contains(NewPin) && MovePin(OldPin, NewPin))
			{
				MovedOldPins.Add(OldPin);
				MovedNewPins.Add(NewPin);
			}
		}
	}
}

void UVoxelGraphNode_Base::ReconstructNode()
{
	Super::ReconstructNode();
	
	if (!ensure(this))
	{
		return;
	}
	Modify();

	// Break any links to 'orphan' pins
	for (auto& Pin : Pins)
	{
		for (auto& OtherPin : Pin->LinkedTo)
		{
			// If we are linked to a pin that its owner doesn't know about, break that link
			if (!OtherPin->GetOwningNode()->Pins.Contains(OtherPin))
			{
				Pin->LinkedTo.Remove(OtherPin);
			}
		}
	}

	// Store the old Input and Output pins
	const TArray<UEdGraphPin*> OldInputPins = GetInputPins();
	const TArray<UEdGraphPin*> OldOutputPins = GetOutputPins();

	// Move the existing pins to a saved array
	const TArray<UEdGraphPin*> OldPins = Pins;
	Pins.Reset();

	// Recreate the new pins
	AllocateDefaultPins();
	// Restore vector pins
	RestoreVectorPins(OldInputPins, OldOutputPins);

	// Get new Input and Output pins
	const TArray<UEdGraphPin*> NewInputPins = GetInputPins();
	const TArray<UEdGraphPin*> NewOutputPins = GetOutputPins();

	MovePins(OldInputPins, NewInputPins);
	MovePins(OldOutputPins, NewOutputPins);

	// Throw away the original pins
	for (UEdGraphPin* OldPin : OldPins)
	{
		OldPin->Modify();
		UEdGraphNode::DestroyPin(OldPin);
	}

	GetGraph()->NotifyGraphChanged();
}

void UVoxelGraphNode_Base::AutowireNewNode(UEdGraphPin* FromPin)
{
	if (FromPin != NULL)
	{
		const UVoxelGraphSchema* Schema = CastChecked<UVoxelGraphSchema>(GetSchema());

		TSet<UEdGraphNode*> NodeList;

		// auto-connect from dragged pin to first compatible pin on the new node
		for (int32 i = 0; i < Pins.Num(); i++)
		{
			UEdGraphPin* Pin = Pins[i];
			check(Pin);
			FPinConnectionResponse Response = Schema->CanCreateConnection(FromPin, Pin);

			if (ECanCreateConnectionResponse::CONNECT_RESPONSE_MAKE == Response.Response)
			{
				if (Schema->TryCreateConnection(FromPin, Pin))
				{
					NodeList.Add(FromPin->GetOwningNode());
					NodeList.Add(this);
				}
				break;
			}
			else if (ECanCreateConnectionResponse::CONNECT_RESPONSE_BREAK_OTHERS_A == Response.Response)
			{
				InsertNewNode(FromPin, Pin, NodeList);
				break;
			}
		}

		// Send all nodes that received a new pin connection a notification
		for (auto It = NodeList.CreateConstIterator(); It; ++It)
		{
			UEdGraphNode* Node = (*It);
			Node->NodeConnectionListChanged();
		}
	}
}

bool UVoxelGraphNode_Base::CanCreateUnderSpecifiedSchema(const UEdGraphSchema* Schema) const
{
	return Schema->IsA(UVoxelGraphSchema::StaticClass());
}