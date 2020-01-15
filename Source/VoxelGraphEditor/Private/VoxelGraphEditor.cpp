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

inline FVoxelPin CreateVoxelPinFromGraphPin(UEdGraphPin* Pin)
{
	TArray<UVoxelNode*> OtherNodes;
	TArray<FGuid> OtherPinIds;

	if (Pin->LinkedTo.Num() > 0)
	{
		for (UEdGraphPin* OtherPin : Pin->LinkedTo)
		{
			auto Knot = Cast<UVoxelGraphNode_Knot>(OtherPin->GetOwningNode());
			if (Knot)
			{
				auto NewOtherPins = Pin->Direction == EEdGraphPinDirection::EGPD_Input ? Knot->GetAllInputPins() : Knot->GetAllOutputPins();
				for (auto& NewOtherPin : NewOtherPins)
				{
					auto* const NewOtherNode = Cast<UVoxelGraphNode>(NewOtherPin->GetOwningNode());
					if (NewOtherNode)
					{
						OtherNodes.Add(NewOtherNode->VoxelNode);
						OtherPinIds.Add(NewOtherPin->PinId);
					}
				}
			}
			else
			{
				auto* const OtherNode = Cast<UVoxelGraphNode>(OtherPin->GetOwningNode());
				if (OtherNode)
				{
					OtherNodes.Add(OtherNode->VoxelNode);
					OtherPinIds.Add(OtherPin->PinId);
				}
			}
		}
	}

	return FVoxelPin(Pin->PinId, Pin->DefaultValue, FVoxelPinCategory::FromString(Pin->PinType.PinCategory), OtherNodes, OtherPinIds);
}

UEdGraph* FVoxelGraphEditor::CreateNewVoxelGraph(UVoxelGraphGenerator* InVoxelWorldGenerator)
{
	return CastChecked<UVoxelEdGraph>(FBlueprintEditorUtils::CreateNewGraph(InVoxelWorldGenerator, NAME_None, UVoxelEdGraph::StaticClass(), UVoxelGraphSchema::StaticClass()));
}

void FVoxelGraphEditor::CreateVoxelGraphNode(UEdGraph* VoxelGraph, UVoxelNode* InVoxelNode, bool bSelectNewNode)
{
	FGraphNodeCreator<UVoxelGraphNode> NodeCreator(*VoxelGraph);
	UVoxelGraphNode* GraphNode = NodeCreator.CreateUserInvokedNode(bSelectNewNode);
	InVoxelNode->GraphNode = GraphNode;
	GraphNode->SetVoxelNode(InVoxelNode);
	NodeCreator.Finalize();
}

void FVoxelGraphEditor::CompileVoxelNodesFromGraphNodes(UVoxelGraphGenerator* WorldGenerator)
{
	WorldGenerator->Modify();
	{
		WorldGenerator->FirstNode = nullptr;
		WorldGenerator->AllNodes.Empty();

		TArray<UVoxelNode*> AllNodes;
		for (auto& Node : WorldGenerator->VoxelGraph->Nodes)
		{
			UVoxelGraphNode* GraphNode = Cast<UVoxelGraphNode>(Node);
			if (GraphNode && GraphNode->VoxelNode)
			{
				UVoxelNode* VoxelNode = GraphNode->VoxelNode;
				check(VoxelNode);

				check(!AllNodes.Contains(VoxelNode));
				AllNodes.Add(VoxelNode);

				TArray<FVoxelPin> InputVoxelPins;
				{
					TArray<UEdGraphPin*> InputPins;
					GraphNode->GetInputPins(InputPins);
					for (auto& InputPin : InputPins)
					{
						InputVoxelPins.Add(CreateVoxelPinFromGraphPin(InputPin));
					}
				}

				TArray<FVoxelPin> OutputVoxelPins;
				{
					TArray<UEdGraphPin*> OutputPins;
					GraphNode->GetOutputPins(OutputPins);
					for (auto& OutputPin : OutputPins)
					{
						OutputVoxelPins.Add(CreateVoxelPinFromGraphPin(OutputPin));
					}
				}

				VoxelNode->SetFlags(RF_Transactional);
				VoxelNode->Modify();
				VoxelNode->InputPins = InputVoxelPins;
				VoxelNode->OutputPins = OutputVoxelPins;
				VoxelNode->PostEditChange();
			}
			else
			{
				UVoxelGraphNode_Root* GraphNodeRoot = Cast<UVoxelGraphNode_Root>(Node);
				if (GraphNodeRoot)
				{
					TArray<UEdGraphPin*> OutputPins;
					GraphNodeRoot->GetOutputPins(OutputPins);

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
								WorldGenerator->FirstNode = CastChecked<UVoxelGraphNode>(NewOtherPin->GetOwningNode())->VoxelNode;
								WorldGenerator->FirstNodePinId = NewOtherPin->PinId;
							}
						}
						else
						{
							WorldGenerator->FirstNode = CastChecked<UVoxelGraphNode>(OtherPin->GetOwningNode())->VoxelNode;
							WorldGenerator->FirstNodePinId = OtherPin->PinId;
						}
					}
				}
			}
		}

		AllNodes.Remove(nullptr);
		WorldGenerator->AllNodes = AllNodes;
	}
	WorldGenerator->PostEditChange();

	UpdatePreview(WorldGenerator, false, true);
}

void FVoxelGraphEditor::UpdatePreview(UVoxelGraphGenerator* WorldGenerator, bool bForce, bool bUpdateTextures)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(WorldGenerator->VoxelGraph))
	{
		Editor->TriggerUpdatePreview(bForce, bUpdateTextures);
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

void FVoxelGraphEditor::AddMessages(const UVoxelGraphGenerator* WorldGenerator, const TArray<FVoxelGraphMessage>& Messages)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(WorldGenerator->VoxelGraph))
	{
		Editor->AddMessages(Messages);
	}
}

void FVoxelGraphEditor::ClearMessages(const UVoxelGraphGenerator* WorldGenerator, bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear)
{
	if (auto Editor = FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(WorldGenerator->VoxelGraph))
	{
		Editor->ClearMessages(bClearAll, MessagesToClear);
	}
}