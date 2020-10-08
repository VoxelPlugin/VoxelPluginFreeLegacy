// Copyright 2020 Phyronnaz

#include "VoxelGraphEditor.h"
#include "VoxelEdGraph.h"
#include "VoxelNode.h"
#include "VoxelGraphSchema.h"
#include "VoxelGraphGenerator.h"
#include "IVoxelGraphEditorToolkit.h"
#include "VoxelGraphEditorUtilities.h"
#include "VoxelGraphNodes/VoxelGraphNode_Root.h"
#include "VoxelGraphNodes/VoxelGraphNode.h"
#include "VoxelGraphNodes/VoxelGraphNode_Knot.h"

#include "Kismet2/BlueprintEditorUtils.h"

inline TArray<FVoxelPin> CreateVoxelPinsFromGraphPin(UEdGraphPin& Pin)
{
	TArray<FVoxelPin> Result;

	if (Pin.SubPins.Num() == 0)
	{
		Result.Add(FVoxelPin(Pin.PinId, Pin.DefaultValue, FVoxelPinCategory::FromString(Pin.PinType.PinCategory)));
	}
	else
	{
		TArray<FString> SubDefaultValues;
		Pin.DefaultValue.ParseIntoArray(SubDefaultValues, TEXT(","));

		for (int32 Index = 0; Index < Pin.SubPins.Num(); Index++)
		{
			auto& SubPin = *Pin.SubPins[Index];
			Result.Add(FVoxelPin(
				SubPin.PinId,
				SubDefaultValues.IsValidIndex(Index) ? SubDefaultValues[Index] : "",
				FVoxelPinCategory::FromString(SubPin.PinType.PinCategory)));
		}
	}

	const auto CheckOtherPin = [&](UEdGraphPin& OtherPin)
	{
		auto* const OtherNode = Cast<UVoxelGraphNode>(OtherPin.GetOwningNode());
		if (!OtherNode)
		{
			return;
		}
		
		if (Pin.SubPins.Num() > 0)
		{
			if (!ensure(Pin.SubPins.Num() == OtherPin.SubPins.Num()))
			{
				return;
			}

			for (int32 Index = 0; Index < Pin.SubPins.Num(); Index++)
			{
				Result[Index].OtherNodes.Add(OtherNode->VoxelNode);
				Result[Index].OtherPinIds.Add(OtherPin.SubPins[Index]->PinId);
			}
		}
		else
		{
			Result[0].OtherNodes.Add(OtherNode->VoxelNode);
			Result[0].OtherPinIds.Add(OtherPin.PinId);
		}
	};

	if (Pin.LinkedTo.Num() > 0)
	{
		for (UEdGraphPin* OtherPin : Pin.LinkedTo)
		{
			auto Knot = Cast<UVoxelGraphNode_Knot>(OtherPin->GetOwningNode());
			if (Knot)
			{
				const auto NewOtherPins = Pin.Direction == EGPD_Input ? Knot->GetAllInputPins() : Knot->GetAllOutputPins();
				for (auto& NewOtherPin : NewOtherPins)
				{
					CheckOtherPin(*NewOtherPin);
				}
			}
			else
			{
				CheckOtherPin(*OtherPin);
			}
		}
	}

	return Result;
}

UEdGraph* FVoxelGraphEditor::CreateNewVoxelGraph(UVoxelGraphGenerator* InGenerator)
{
	return CastChecked<UVoxelEdGraph>(FBlueprintEditorUtils::CreateNewGraph(InGenerator, NAME_None, UVoxelEdGraph::StaticClass(), UVoxelGraphSchema::StaticClass()));
}

void FVoxelGraphEditor::CreateVoxelGraphNode(UEdGraph* VoxelGraph, UVoxelNode* InVoxelNode, bool bSelectNewNode)
{
	FGraphNodeCreator<UVoxelGraphNode> NodeCreator(*VoxelGraph);
	UVoxelGraphNode* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	InVoxelNode->GraphNode = GraphNode;
	GraphNode->SetVoxelNode(InVoxelNode);
	NodeCreator.Finalize();
}

void FVoxelGraphEditor::CompileVoxelNodesFromGraphNodes(UVoxelGraphGenerator* Generator)
{
	Generator->Modify();
	{
		Generator->FirstNode = nullptr;
		Generator->AllNodes.Empty();

		TArray<UVoxelNode*> AllNodes;
		for (auto& Node : Generator->VoxelGraph->Nodes)
		{
			UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(Node);
			if (GraphNode && GraphNode->VoxelNode)
			{
				UVoxelNode* VoxelNode = GraphNode->VoxelNode;
				check(VoxelNode);

				check(!AllNodes.Contains(VoxelNode));
				AllNodes.Add(VoxelNode);

				TArray<FVoxelPin> InputPins;
				for (auto& InputPin : GraphNode->GetInputPins())
				{
					if (!InputPin->bHidden)
					{
						InputPins.Append(CreateVoxelPinsFromGraphPin(*InputPin));
					}
				}

				TArray<FVoxelPin> OutputPins;
				for (auto& OutputPin : GraphNode->GetOutputPins())
				{
					if (!OutputPin->bHidden)
					{
						OutputPins.Append(CreateVoxelPinsFromGraphPin(*OutputPin));
					}
				}

				VoxelNode->SetFlags(RF_Transactional);
				VoxelNode->Modify();
				VoxelNode->InputPins = InputPins;
				VoxelNode->OutputPins = OutputPins;
				VoxelNode->PostEditChange();
			}
			else
			{
				UVoxelGraphNode_Root* GraphNodeRoot = Cast<UVoxelGraphNode_Root>(Node);
				if (GraphNodeRoot)
				{
					const TArray<UEdGraphPin*> OutputPins = GraphNodeRoot->GetOutputPins();

					check(OutputPins.Num() == 1);
					check(OutputPins[0]->LinkedTo.Num() <= 1);
					if (OutputPins[0]->LinkedTo.Num() == 1)
					{
						UEdGraphPin* OtherPin = OutputPins[0]->LinkedTo[0];

						auto Knot = Cast<UVoxelGraphNode_Knot>(OtherPin->GetOwningNode());

						if (Knot)
						{
							auto NewOtherPins = Knot->GetAllOutputPins();
							if (NewOtherPins.Num() > 0)
							{
								check(NewOtherPins.Num() == 1);
								auto NewOtherPin = NewOtherPins[0];
								Generator->FirstNode = CastChecked<UVoxelGraphNode>(NewOtherPin->GetOwningNode())->VoxelNode;
								Generator->FirstNodePinId = NewOtherPin->PinId;
							}
						}
						else
						{
							Generator->FirstNode = CastChecked<UVoxelGraphNode>(OtherPin->GetOwningNode())->VoxelNode;
							Generator->FirstNodePinId = OtherPin->PinId;
						}
					}
				}
			}
		}

		AllNodes.Remove(nullptr);
		Generator->AllNodes = AllNodes;
	}
	Generator->PostEditChange();

	UpdatePreview(Generator, EVoxelGraphPreviewFlags::UpdateTextures);
}

void FVoxelGraphEditor::UpdatePreview(UVoxelGraphGenerator* Generator, EVoxelGraphPreviewFlags Flags)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(Generator->VoxelGraph))
	{
		Editor->TriggerUpdatePreview(Flags);
	}
}

void FVoxelGraphEditor::SelectNodesAndZoomToFit(UEdGraph* Graph, const TArray<UEdGraphNode*>& Nodes)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(Graph))
	{
		Editor->SelectNodesAndZoomToFit(Nodes);
	}
}

void FVoxelGraphEditor::RefreshNodesMessages(UEdGraph* Graph)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(Graph))
	{
		Editor->RefreshNodesMessages();
	}
}

void FVoxelGraphEditor::DebugNodes(UEdGraph* DebugGraph, const TSet<FVoxelCompilationNode*>& Nodes)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(DebugGraph))
	{
		Editor->DebugNodes(Nodes);
	}
}

void FVoxelGraphEditor::AddMessages(const UVoxelGraphGenerator* Generator, const TArray<FVoxelGraphMessage>& Messages)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(Generator->VoxelGraph))
	{
		Editor->AddMessages(Messages);
	}
}

void FVoxelGraphEditor::ClearMessages(const UVoxelGraphGenerator* Generator, bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(Generator->VoxelGraph))
	{
		Editor->ClearMessages(bClearAll, MessagesToClear);
	}
}