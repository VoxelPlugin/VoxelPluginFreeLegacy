// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelGraphSchema.h"
#include "VoxelEdGraph.h"
#include "VoxelGraphEditorUtilities.h"
#include "VoxelGraphEditorCommands.h"
#include "IVoxelGraphEditorToolkit.h"

#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "VoxelAssets/VoxelDataAsset.h"

#include "VoxelNode.h"
#include "VoxelNodes/VoxelGraphMacro.h"
#include "VoxelNodes/VoxelLocalVariables.h"
#include "VoxelNodes/VoxelExecNodes.h"
#include "VoxelNodes/VoxelHeightmapSamplerNode.h"
#include "VoxelNodes/VoxelDataAssetSamplerNode.h"
#include "VoxelNodes/VoxelTextureSamplerNode.h"
#include "VoxelNodes/VoxelCurveNodes.h"
#include "VoxelNodes/VoxelMathNodes.h"
#include "VoxelNodes/VoxelGeneratorSamplerNodes.h"

#include "VoxelGraphNodes/VoxelGraphNode_Knot.h"
#include "VoxelGraphNodes/VoxelGraphNode.h"
#include "VoxelGraphNodes/VoxelGraphNode_Root.h"

#include "Engine/Texture2D.h"
#include "ScopedTransaction.h"
#include "EdGraphNode_Comment.h"
#include "GraphEditorSettings.h"
#include "Layout/SlateRect.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Commands/GenericCommands.h"
#include "UObject/UObjectIterator.h"
#include "GraphEditorActions.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Engine/StreamableManager.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"

#include "ToolMenu.h"
#include "ToolMenuSection.h"

UEdGraphNode* FVoxelGraphSchemaAction_NewNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	check(VoxelNodeClass);
	UVoxelGraphGenerator* Generator = CastChecked<UVoxelEdGraph>(ParentGraph)->GetGenerator();

	const FScopedTransaction Transaction(VOXEL_LOCTEXT("New voxel node"));

	UVoxelNode* NewNode = Generator->ConstructNewNode(VoxelNodeClass, FVector2D(Location), bSelectNewNode);
	NewNode->GraphNode->ReconstructNode();

	// Autowire before combining if not vector
	if (FromPin && FVoxelPinCategory::FromString(FromPin->PinType.PinCategory) != EVoxelPinCategory::Vector)
	{
		NewNode->GraphNode->AutowireNewNode(FromPin);
	}

	// Combine all vector pins on spawn
	if (auto* VoxelNode = Cast<UVoxelGraphNode>(NewNode->GraphNode))
	{
		VoxelNode->CombineAll();
	}
	
	// Autowire after combining if vector
	if (FromPin && FVoxelPinCategory::FromString(FromPin->PinType.PinCategory) == EVoxelPinCategory::Vector)
	{
		NewNode->GraphNode->AutowireNewNode(FromPin);
	}
	
	// Else the voxel pin arrays are invalid
	Generator->CompileVoxelNodesFromGraphNodes();

	return NewNode->GraphNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewMacroNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	check(Macro);
	UVoxelGraphGenerator* Generator = CastChecked<UVoxelEdGraph>(ParentGraph)->GetGenerator();

	const FScopedTransaction Transaction(VOXEL_LOCTEXT("New macro node"));

	UVoxelGraphMacroNode* NewNode = Generator->ConstructNewNode<UVoxelGraphMacroNode>(FVector2D(Location), bSelectNewNode);
	NewNode->Macro = Macro;
	NewNode->GraphNode->ReconstructNode();

	// Autowire before combining if not vector
	if (FromPin && FVoxelPinCategory::FromString(FromPin->PinType.PinCategory) != EVoxelPinCategory::Vector)
	{
		NewNode->GraphNode->AutowireNewNode(FromPin);
	}

	// Combine all vector pins on spawn
	if (auto* VoxelNode = Cast<UVoxelGraphNode>(NewNode->GraphNode))
	{
		VoxelNode->CombineAll();
	}
	
	// Autowire after combining if vector
	if (FromPin && FVoxelPinCategory::FromString(FromPin->PinType.PinCategory) == EVoxelPinCategory::Vector)
	{
		NewNode->GraphNode->AutowireNewNode(FromPin);
	}

	// Else the voxel pin arrays are invalid
	Generator->CompileVoxelNodesFromGraphNodes();

	return NewNode->GraphNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewLocalVariableDeclaration::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	UVoxelGraphGenerator* Generator = CastChecked<UVoxelEdGraph>(ParentGraph)->GetGenerator();

	const FScopedTransaction Transaction(VOXEL_LOCTEXT("New local variable declaration"));

	UVoxelLocalVariableDeclaration* Declaration = Generator->ConstructNewNode<UVoxelLocalVariableDeclaration>(FVector2D(Location), bSelectNewNode);
	Declaration->SetCategory(PinCategory);

	if (!DefaultName.IsNone())
	{
		Declaration->Name = DefaultName;
	}

	Declaration->GraphNode->ReconstructNode();
	Declaration->GraphNode->AutowireNewNode(FromPin);

	// Else the voxel pin arrays are invalid
	Generator->CompileVoxelNodesFromGraphNodes();

	return Declaration->GraphNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewLocalVariableUsage::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	check(Declaration);
	UVoxelGraphGenerator* Generator = CastChecked<UVoxelEdGraph>(ParentGraph)->GetGenerator();

	const FScopedTransaction Transaction(VOXEL_LOCTEXT("New local variable usage"));

	UVoxelLocalVariableUsage* Usage = Generator->ConstructNewNode<UVoxelLocalVariableUsage>(FVector2D(Location), bSelectNewNode);
	Usage->Declaration = Declaration;
	Usage->DeclarationGuid = Declaration->VariableGuid;

	Usage->GraphNode->ReconstructNode();
	Usage->GraphNode->AutowireNewNode(FromPin);

	// Else the voxel pin arrays are invalid
	Generator->CompileVoxelNodesFromGraphNodes();

	return Usage->GraphNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewSetterNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	UVoxelGraphGenerator* Generator = CastChecked<UVoxelEdGraph>(ParentGraph)->GetGenerator();

	const FScopedTransaction Transaction(VOXEL_LOCTEXT("New setter node"));

	UVoxelNode_SetNode* NewNode = Generator->ConstructNewNode<UVoxelNode_SetNode>(FVector2D(Location), bSelectNewNode);
	NewNode->SetIndex(Index);
	NewNode->GraphNode->ReconstructNode();
	NewNode->GraphNode->AutowireNewNode(FromPin);

	// Else the voxel pin arrays are invalid
	Generator->CompileVoxelNodesFromGraphNodes();

	return NewNode->GraphNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewKnotNode::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	const FScopedTransaction Transaction(VOXEL_LOCTEXT("New reroute node"));
	ParentGraph->Modify();

	FGraphNodeCreator<UVoxelGraphNode_Knot> KnotNodeCreator(*ParentGraph);
	UVoxelGraphNode_Knot* KnotNode = KnotNodeCreator.CreateNode(bSelectNewNode);
	KnotNodeCreator.Finalize();

	KnotNode->NodePosX = Location.X;
	KnotNode->NodePosY = Location.Y;
	
	KnotNode->AutowireNewNode(FromPin);
	KnotNode->PropagatePinType();

	return KnotNode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_NewComment::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	// Add menu item for creating comment boxes
	UEdGraphNode_Comment* CommentTemplate = NewObject<UEdGraphNode_Comment>();

	FVector2D SpawnLocation(Location);

	FSlateRect Bounds;
	if (FVoxelGraphEditorUtilities::GetBoundsForSelectedNodes(ParentGraph, Bounds, 50.0f))
	{
		CommentTemplate->SetBounds(Bounds);
		SpawnLocation.X = CommentTemplate->NodePosX;
		SpawnLocation.Y = CommentTemplate->NodePosY;
	}

	return FEdGraphSchemaAction_NewNode::SpawnNodeFromTemplate<UEdGraphNode_Comment>(ParentGraph, CommentTemplate, SpawnLocation);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

UEdGraphNode* FVoxelGraphSchemaAction_Paste::PerformAction(UEdGraph* ParentGraph, UEdGraphPin* FromPin, const UE_506_SWITCH(FVector2D, FVector2f&) Location, bool bSelectNewNode)
{
	FVoxelGraphEditorUtilities::PasteNodesHere(ParentGraph, FVector2D(Location));
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

bool UVoxelGraphSchema::ConnectionCausesLoop(const UEdGraphPin* InputPin, const UEdGraphPin* OutputPin) const
{
	UEdGraphNode* const StartNode = OutputPin->GetOwningNode();

	TSet<UEdGraphNode*> ProcessedNodes;

	TArray<UEdGraphNode*> NodesToProcess;
	NodesToProcess.Add(InputPin->GetOwningNode());

	while (NodesToProcess.Num() > 0)
	{
		UEdGraphNode* Node = NodesToProcess.Pop(UE_505_SWITCH(false, EAllowShrinking::No));
		if (ProcessedNodes.Contains(Node))
		{
			continue;
		}
		ProcessedNodes.Add(Node);

		if (auto* PortalInputGraphNode = Cast<UVoxelGraphNode>(Node))
		{
			if (auto* Declaration = Cast<UVoxelLocalVariableDeclaration>(PortalInputGraphNode->VoxelNode))
			{
				if (!ensure(Declaration->Graph))
				{
					continue;
				}
				for (UVoxelNode* OtherNode : Declaration->Graph->AllNodes)
				{
					auto* Usage = Cast<UVoxelLocalVariableUsage>(OtherNode);
					if (Usage && Usage->Declaration == Declaration)
					{
						if (StartNode == Usage->GraphNode)
						{
							return true;
						}
						NodesToProcess.Add(Usage->GraphNode);
					}
				}
			}
		}

		for (UEdGraphPin* Pin : Node->GetAllPins())
		{
			if (Pin->Direction == EGPD_Output)
			{
				for (auto& LPin : Pin->LinkedTo)
				{
					check(LPin->Direction == EGPD_Input);

					UEdGraphNode* NewNode = LPin->GetOwningNode();
					check(NewNode);

					if (StartNode == NewNode)
					{
						return true;
					}
					NodesToProcess.Add(NewNode);
				}
			}
		}
	}

	return false;
}

void UVoxelGraphSchema::GetPaletteActions(FGraphActionMenuBuilder& ActionMenuBuilder) const
{
	GetAllVoxelNodeActions(ActionMenuBuilder);
	GetCommentAction(ActionMenuBuilder);
}

void UVoxelGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	GetAllVoxelNodeActions(ContextMenuBuilder, ContextMenuBuilder.CurrentGraph);
	GetCommentAction(ContextMenuBuilder, ContextMenuBuilder.CurrentGraph);

	if (!ContextMenuBuilder.FromPin && FVoxelGraphEditorUtilities::CanPasteNodes(ContextMenuBuilder.CurrentGraph))
	{
		TSharedPtr<FVoxelGraphSchemaAction_Paste> NewAction(new FVoxelGraphSchemaAction_Paste(FText::GetEmpty(), VOXEL_LOCTEXT("Paste here"), FText::GetEmpty(), 0));
		ContextMenuBuilder.AddAction(NewAction);
	}
}

bool UVoxelGraphSchema::TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const
{
	bool bModified = UEdGraphSchema::TryCreateConnection(A, B);

	auto AK = Cast<UVoxelGraphNode_Knot>(A->GetOwningNode());
	auto BK = Cast<UVoxelGraphNode_Knot>(B->GetOwningNode());
	if (AK)
	{
		AK->PropagatePinType();
	}
	if (BK)
	{
		BK->PropagatePinType();
	}

	if (bModified)
	{
		CastChecked<UVoxelEdGraph>(A->GetOwningNode()->GetGraph())->GetGenerator()->CompileVoxelNodesFromGraphNodes();
	}

	return bModified;
}

void UVoxelGraphSchema::TrySetDefaultValue(
	UEdGraphPin& Pin,
	const FString& NewDefaultValue, 
	bool bMarkAsModified
) const
{
	FString DefaultValue = NewDefaultValue;

	auto Node = Cast<UVoxelGraphNode>(Pin.GetOwningNode());
	if (Node && Node->VoxelNode)
	{
		int32 Index = Node->GetInputPinIndex(&Pin);
		if (Index >= 0)
		{
			auto Category = FVoxelPinCategory::FromString(Pin.PinType.PinCategory);
			if (Category == EVoxelPinCategory::Float)
			{
				float Value = FCString::Atof(*DefaultValue);
				auto Bounds = Node->VoxelNode->GetInputPinDefaultValueBounds(Index);

				if (Bounds.Min.IsSet())
				{
					Value = FMath::Max(Bounds.Min.GetValue(), Value);
				}
				if (Bounds.Max.IsSet())
				{
					Value = FMath::Min(Bounds.Max.GetValue(), Value);
				}

				DefaultValue = FString::SanitizeFloat(Value);
			}
			else if (Category == EVoxelPinCategory::Int)
			{
				int32 Value = FCString::Atoi(*DefaultValue);
				auto Bounds = Node->VoxelNode->GetInputPinDefaultValueBounds(Index);

				if (Bounds.Min.IsSet())
				{
					Value = FMath::Max(FMath::RoundToInt(Bounds.Min.GetValue()), Value);
				}
				if (Bounds.Max.IsSet())
				{
					Value = FMath::Min(FMath::RoundToInt(Bounds.Max.GetValue()), Value);
				}

				DefaultValue = FString::FromInt(Value);
			}
		}
	}

	Super::TrySetDefaultValue(Pin, DefaultValue);

	CastChecked<UVoxelEdGraph>(Pin.GetOwningNode()->GetGraph())->GetGenerator()->CompileVoxelNodesFromGraphNodes();
}

bool UVoxelGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	if (PinA->Direction == EGPD_Input)
	{
		//Swap so that A is the from pin and B is the to pin.
		UEdGraphPin* Temp = PinA;
		PinA = PinB;
		PinB = Temp;
	}

	EVoxelPinCategory AType = FVoxelPinCategory::FromString(PinA->PinType.PinCategory);
	EVoxelPinCategory BType = FVoxelPinCategory::FromString(PinB->PinType.PinCategory);

	if (AType != BType && (AType == EVoxelPinCategory::Float || AType == EVoxelPinCategory::Int) && (BType == EVoxelPinCategory::Float || BType == EVoxelPinCategory::Int))
	{
		UEdGraphNode* ANode = PinA->GetOwningNode();
		UEdGraphNode* BNode = PinB->GetOwningNode();
		UVoxelEdGraph* Graph = CastChecked<UVoxelEdGraph>(ANode->GetGraph());
		UVoxelGraphGenerator* Generator = Graph->GetGenerator();

		// Since we'll be adding a node, make sure to modify the graph itself.
		Graph->Modify();
		UVoxelNode* ConvertNode;
		FVector2D Position((ANode->NodePosX + BNode->NodePosX) / 2, (ANode->NodePosY + BNode->NodePosY) / 2);
		if (AType == EVoxelPinCategory::Int)
		{
			ConvertNode = Generator->ConstructNewNode<UVoxelNode_FloatOfInt>(Position, false);
		}
		else
		{
			ConvertNode = Generator->ConstructNewNode<UVoxelNode_Round>(Position, false);
		}

		UVoxelGraphNode* ConvertGraphNode = CastChecked<UVoxelGraphNode>(ConvertNode->GraphNode);

		auto InputPin = ConvertGraphNode->GetInputPin(0);
		auto OutputPin = ConvertGraphNode->GetOutputPin(0);

		check(InputPin->PinType.PinCategory == PinA->PinType.PinCategory);
		check(OutputPin->PinType.PinCategory == PinB->PinType.PinCategory);

		if (!UEdGraphSchema::TryCreateConnection(PinA, InputPin))
		{
			Graph->RemoveNode(ConvertGraphNode);
			return false;
		}
		if (!UEdGraphSchema::TryCreateConnection(OutputPin, PinB))
		{
			InputPin->BreakAllPinLinks();
			Graph->RemoveNode(ConvertGraphNode);
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

TArray<UClass*> UVoxelGraphSchema::VoxelNodeClasses;
bool UVoxelGraphSchema::bVoxelNodeClassesInitialized = false;

FLinearColor UVoxelGraphSchema::GetPinTypeColor(const FEdGraphPinType& PinType) const
{
	EVoxelPinCategory Category = FVoxelPinCategory::FromString(PinType.PinCategory);
	const UGraphEditorSettings* Settings = GetDefault<UGraphEditorSettings>();
	
	if (Category == EVoxelPinCategory::Exec)
	{
		return Settings->ExecutionPinTypeColor;
	}
	else if (Category == EVoxelPinCategory::Float)
	{
		return Settings->FloatPinTypeColor;
	}
	else if (Category == EVoxelPinCategory::Boolean)
	{
		return Settings->BooleanPinTypeColor;
	}
	else if (Category == EVoxelPinCategory::Int)
	{
		return Settings->IntPinTypeColor;
	}
	else if (Category == EVoxelPinCategory::Material)
	{
		return Settings->ObjectPinTypeColor;
	}
	else if (Category == EVoxelPinCategory::Color)
	{
		return Settings->StructPinTypeColor;
	}
	else if (Category == EVoxelPinCategory::Seed)
	{
		return Settings->SoftClassPinTypeColor;
	}
	else if (Category == EVoxelPinCategory::Vector)
	{
		return Settings->VectorPinTypeColor;
	}

	// Type does not have a defined color!
	return Settings->DefaultPinTypeColor;
}

TSharedPtr<FEdGraphSchemaAction> UVoxelGraphSchema::GetCreateCommentAction() const
{
	return TSharedPtr<FEdGraphSchemaAction>(static_cast<FEdGraphSchemaAction*>(new FVoxelGraphSchemaAction_NewComment));
}

int32 UVoxelGraphSchema::GetNodeSelectionCount(const UEdGraph* Graph) const
{
	return FVoxelGraphEditorUtilities::GetNumberOfSelectedNodes(Graph);
}

void UVoxelGraphSchema::GetContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	const UEdGraph* CurrentGraph = Context->Graph;
	const UEdGraphNode* InGraphNode = Context->Node;
	const UEdGraphPin* InGraphPin = Context->Pin;

	if (InGraphPin)
	{
		FToolMenuSection& Section = Menu->AddSection("MaterialGraphSchemaPinActions", VOXEL_LOCTEXT("Pin Actions"));
		if (InGraphPin->LinkedTo.Num() > 0)
		{
			Section.AddMenuEntry(FGraphEditorCommands::Get().BreakPinLinks);
		}

		if (auto* Node = Cast<UVoxelGraphNode>(InGraphNode))
		{
			if (UVoxelNode* VoxelNode = Node->VoxelNode)
			{
				// If on an input that can be deleted, show option
				if (InGraphPin->Direction == EGPD_Input)
				{
					const int32 MinPins = VoxelNode->GetMinInputPins();
					const int32 MaxPins = VoxelNode->GetMaxInputPins();
					if (MinPins != MaxPins && MinPins < VoxelNode->InputPinCount)
					{
						Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().DeleteInput);
					}
				}
				// Preview
				if (InGraphPin->Direction == EGPD_Output && FVoxelPinCategory::FromString(InGraphPin->PinType.PinCategory) == EVoxelPinCategory::Float)
				{
					Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().TogglePinPreview);
				}
				if (Node->CanSplitPin_Voxel(*InGraphPin))
				{
					Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().SplitPin);
				}
				if (Node->CanCombinePin(*InGraphPin))
				{
					Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().CombinePin);
				}
			}
		}
	}
	else if (InGraphNode)
	{
		// Add a 'Convert to Local Variables' option to reroute nodes
		if (auto* Knot = Cast<UVoxelGraphNode_Knot>(InGraphNode))
		{
			const EVoxelPinCategory Category = FVoxelPinCategory::FromString(Knot->GetInputPin()->PinType.PinCategory);
			if (Category != EVoxelPinCategory::Exec &&
				Category != EVoxelPinCategory::Wildcard &&
				Category != EVoxelPinCategory::Vector)
			{
				FToolMenuSection& Section = Menu->AddSection("MaterialEditorMenu1");
				Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().ConvertRerouteToVariables);
			}
		}

		if (auto* Node = Cast<UVoxelGraphNode>(InGraphNode))
		{
			if (UVoxelNode* VoxelNode = Node->VoxelNode)
			{
				// Add local variables selection & conversion to reroute nodes
				if (VoxelNode->IsA(UVoxelLocalVariableBase::StaticClass()))
				{
					FToolMenuSection& Section = Menu->AddSection("MaterialEditorMenu1");
					if (VoxelNode->IsA(UVoxelLocalVariableDeclaration::StaticClass()))
					{
						Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().SelectLocalVariableUsages);
					}
					if (VoxelNode->IsA(UVoxelLocalVariableUsage::StaticClass()))
					{
						Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().SelectLocalVariableDeclaration);
					}
					Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().ConvertVariablesToReroute);
				}
			}
		}

		{
			FToolMenuSection& Section = Menu->AddSection("VoxelGraphNodeEdit");
			Section.AddMenuEntry(FGenericCommands::Get().Delete);
			Section.AddMenuEntry(FGenericCommands::Get().Cut);
			Section.AddMenuEntry(FGenericCommands::Get().Copy);
			Section.AddMenuEntry(FGenericCommands::Get().Duplicate);
		}
		{
			FToolMenuSection& Section = Menu->AddSection("VoxelGraphNodeMisc");
			Section.AddMenuEntry(FVoxelGraphEditorCommands::Get().ReconstructNode);
		}

		{
			FToolMenuSection& Section = Menu->AddSection("VoxelGraphNodeAligment");
			Section.AddSubMenu("Alignment", VOXEL_LOCTEXT("Alignment"), FText(), FNewMenuDelegate::CreateLambda([](FMenuBuilder& InMenuBuilder) {

				InMenuBuilder.BeginSection("EdGraphSchemaAlignment", VOXEL_LOCTEXT("Align"));
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesTop);
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesMiddle);
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesBottom);
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesLeft);
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesCenter);
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().AlignNodesRight);
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().StraightenConnections);
				InMenuBuilder.EndSection();

				InMenuBuilder.BeginSection("EdGraphSchemaDistribution", VOXEL_LOCTEXT("Distribution"));
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().DistributeNodesHorizontally);
				InMenuBuilder.AddMenuEntry(FGraphEditorCommands::Get().DistributeNodesVertically);
				InMenuBuilder.EndSection();
				}));
		}
	}
}

void UVoxelGraphSchema::DroppedAssetsOnGraph(const TArray<FAssetData>& Assets, const UE_506_SWITCH(FVector2D, FVector2f)& GraphPosition, UEdGraph* Graph) const
{
	auto* Generator = CastChecked<UVoxelEdGraph>(Graph)->GetGenerator();
	FStreamableManager AssetLoader;
	for(auto& AssetData : Assets)
	{
		UObject* Asset = AssetLoader.LoadSynchronous(AssetData.GetSoftObjectPath());
		if (Asset->IsA<UVoxelHeightmapAsset>())
		{
			auto* Node = Generator->ConstructNewNode<UVoxelNode_HeightmapSampler>(FVector2D(GraphPosition));
			if (Asset->IsA<UVoxelHeightmapAssetFloat>())
			{
				Node->bFloatHeightmap = true;
				Node->HeightmapFloat = CastChecked<UVoxelHeightmapAssetFloat>(Asset);
			}
			else if (ensure(Asset->IsA<UVoxelHeightmapAssetUINT16>()))
			{
				Node->bFloatHeightmap = false;
				Node->HeightmapUINT16 = CastChecked<UVoxelHeightmapAssetUINT16>(Asset);
			}
			Node->SetEditableName(Asset->GetName());
		}
		else if (Asset->IsA<UVoxelDataAsset>())
		{
			auto* Node = Generator->ConstructNewNode<UVoxelNode_DataAssetSampler>(FVector2D(GraphPosition));
			Node->Asset = CastChecked<UVoxelDataAsset>(Asset);
			Node->SetEditableName(Asset->GetName());
		}
		else if (Asset->IsA<UTexture2D>())
		{
			auto* Node = Generator->ConstructNewNode<UVoxelNode_TextureSampler>(FVector2D(GraphPosition));
			Node->Texture = CastChecked<UTexture2D>(Asset);
			Node->SetEditableName(Asset->GetName());
		}
		else if (Asset->IsA<UCurveFloat>())
		{
			auto* Node = Generator->ConstructNewNode<UVoxelNode_Curve>(FVector2D(GraphPosition));
			Node->Curve = CastChecked<UCurveFloat>(Asset);
			Node->SetEditableName(Asset->GetName());
		}
		else if (Asset->IsA<UCurveLinearColor>())
		{
			auto* Node = Generator->ConstructNewNode<UVoxelNode_CurveColor>(FVector2D(GraphPosition));
			Node->Curve = CastChecked<UCurveLinearColor>(Asset);
			Node->SetEditableName(Asset->GetName());
		}
		else if (Asset->IsA<UVoxelGraphMacro>())
		{
			auto* Node = Generator->ConstructNewNode<UVoxelGraphMacroNode>(FVector2D(GraphPosition));
			Node->Macro = CastChecked<UVoxelGraphMacro>(Asset);
			Node->GraphNode->ReconstructNode();
			Node->SetEditableName(Asset->GetName());
		}
		else if (Asset->IsA<UVoxelGenerator>())
		{
			auto* Node = Generator->ConstructNewNode<UVoxelNode_GetGeneratorValue>(FVector2D(GraphPosition));
			Node->Generator = CastChecked<UVoxelGenerator>(Asset);
			Node->SetEditableName(Asset->GetName());
		}
	}
	Generator->CompileVoxelNodesFromGraphNodes();
}

void UVoxelGraphSchema::GetAssetsGraphHoverMessage(const TArray<FAssetData>& Assets, const UEdGraph* HoverGraph, FString& OutTooltipText, bool& OutOkIcon) const
{
	FStreamableManager AssetLoader;
	for(auto& AssetData : Assets)
	{
		UObject* Asset = AssetLoader.LoadSynchronous(AssetData.GetSoftObjectPath());
		if (Asset->IsA<UVoxelHeightmapAsset>())
		{
			OutOkIcon = true;
			OutTooltipText = "Add Heightmap Sampler node";
		}
		else if (Asset->IsA<UTexture2D>())
		{
			OutOkIcon = true;
			OutTooltipText = "Add Texture Sampler node";
		}
		else if (Asset->IsA<UCurveFloat>())
		{
			OutOkIcon = true;
			OutTooltipText = "Add Curve Sampler node";
		}
		else if (Asset->IsA<UCurveLinearColor>())
		{
			OutOkIcon = true;
			OutTooltipText = "Add Color Curve Sampler node";
		}
		else if (Asset->IsA<UVoxelGraphMacro>())
		{
			OutOkIcon = true;
			OutTooltipText = "Add Macro node";
		}
		else if (Asset->IsA<UVoxelGenerator>())
		{
			OutOkIcon = true;
			OutTooltipText = "Add Generator Sampler node";
		}
	}
}

void UVoxelGraphSchema::CreateDefaultNodesForGraph(UEdGraph& Graph) const
{
	FGraphNodeCreator<UVoxelGraphNode_Root> StartNodeCreator(Graph);
	UVoxelGraphNode_Root* StartNode = StartNodeCreator.CreateNode();
	StartNodeCreator.Finalize();
	SetNodeMetaData(StartNode, FNodeMetadata::DefaultGraphNode);
	
	UVoxelGraphMacro* Macro = Cast<UVoxelGraphMacro>(CastChecked<UVoxelEdGraph>(&Graph)->GetGenerator());
	if (Macro)
	{
		UVoxelGraphMacroInputNode* InputNode = Macro->ConstructNewNode<UVoxelGraphMacroInputNode>(FVector2D(-100, 0));
		UVoxelGraphMacroOutputNode* OutputNode = Macro->ConstructNewNode<UVoxelGraphMacroOutputNode>(FVector2D(100, 0));

		Macro->InputNode = InputNode;
		Macro->OutputNode = OutputNode;

		SetNodeMetaData(InputNode->GraphNode, FNodeMetadata::DefaultGraphNode);
		SetNodeMetaData(OutputNode->GraphNode, FNodeMetadata::DefaultGraphNode);

		StartNode->NodePosX = -500;
	}
}

void UVoxelGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	Super::BreakNodeLinks(TargetNode);

	CastChecked<UVoxelEdGraph>(TargetNode.GetGraph())->GetGenerator()->CompileVoxelNodesFromGraphNodes();
}

void UVoxelGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	const FScopedTransaction Transaction(VOXEL_LOCTEXT("Break Pin Links"));

	auto OldLinkedTo = TargetPin.LinkedTo;
	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);

	// if this would notify the node then we need to compile the generator
	if (bSendsNodeNotifcation)
	{
		CastChecked<UVoxelEdGraph>(TargetPin.GetOwningNode()->GetGraph())->GetGenerator()->CompileVoxelNodesFromGraphNodes();
	}	

	auto AK = Cast<UVoxelGraphNode_Knot>(TargetPin.GetOwningNode());
	if (AK)
	{
		AK->PropagatePinType();
	}
	for (auto& Pin : OldLinkedTo)
	{
		auto BK = Cast<UVoxelGraphNode_Knot>(Pin->GetOwningNode());
		if (BK)
		{
			BK->PropagatePinType();
		}
	}
}

void UVoxelGraphSchema::OnPinConnectionDoubleCicked(UEdGraphPin* PinA, UEdGraphPin* PinB, const UE_506_SWITCH(FVector2D, FVector2f)& GraphPosition) const
{
	const FScopedTransaction Transaction(VOXEL_LOCTEXT("Create Reroute Node"));

	const UE_506_SWITCH(FVector2D, FVector2f) NodeSpacerSize(42.0f, 24.0f);
	const UE_506_SWITCH(FVector2D, FVector2f) KnotTopLeft = GraphPosition - (NodeSpacerSize * 0.5f);

	// Create a new knot
	UEdGraph* ParentGraph = PinA->GetOwningNode()->GetGraph();

	FVoxelGraphSchemaAction_NewKnotNode Action;
	UVoxelGraphNode_Knot* NewKnot = Cast<UVoxelGraphNode_Knot>(Action.PerformAction(ParentGraph, NULL, KnotTopLeft, true));

	// Move the connections across (only notifying the knot, as the other two didn't really change)
	PinA->BreakLinkTo(PinB);
	PinA->MakeLinkTo((PinA->Direction == EGPD_Output) ? NewKnot->GetInputPin() : NewKnot->GetOutputPin());
	PinB->MakeLinkTo((PinB->Direction == EGPD_Output) ? NewKnot->GetInputPin() : NewKnot->GetOutputPin());
	NewKnot->PropagatePinType();

	// Recompile
	CastChecked<UVoxelEdGraph>(PinA->GetOwningNode()->GetGraph())->GetGenerator()->CompileVoxelNodesFromGraphNodes();
}

const FPinConnectionResponse UVoxelGraphSchema::CanCreateConnection(const UEdGraphPin* PinA, const UEdGraphPin* PinB) const
{
	// Make sure the pins are not on the same node
	if (PinA->GetOwningNode() == PinB->GetOwningNode())
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, VOXEL_LOCTEXT("Both are on the same node"));
	}

	// Compare the directions
	const UEdGraphPin* InputPin = NULL;
	const UEdGraphPin* OutputPin = NULL;

	if (!CategorizePinsByDirection(PinA, PinB, /*out*/ InputPin, /*out*/ OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, VOXEL_LOCTEXT("Directions are not compatible"));
	}

	check(InputPin);
	check(OutputPin);
	auto InputCategory = FVoxelPinCategory::FromString(InputPin->PinType.PinCategory);
	auto OutputCategory = FVoxelPinCategory::FromString(OutputPin->PinType.PinCategory);

	if (InputCategory != OutputCategory && InputCategory != EVoxelPinCategory::Wildcard && OutputCategory != EVoxelPinCategory::Wildcard)
	{
		if (InputCategory == EVoxelPinCategory::Float && OutputCategory == EVoxelPinCategory::Int)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, VOXEL_LOCTEXT("Cast to float"));
		}
		else if (InputCategory == EVoxelPinCategory::Int && OutputCategory == EVoxelPinCategory::Float)
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_MAKE_WITH_CONVERSION_NODE, VOXEL_LOCTEXT("Round to int"));
		}
		else
		{
			return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, VOXEL_LOCTEXT("Types are not compatible"));
		}
	}

	if (ConnectionCausesLoop(InputPin, OutputPin))
	{
		return FPinConnectionResponse(CONNECT_RESPONSE_DISALLOW, VOXEL_LOCTEXT("Connection would cause loop"));
	}

	// Break existing connections on inputs only except for exec - multiple output connections are acceptable
	if (InputCategory != EVoxelPinCategory::Exec && InputPin->LinkedTo.Num() > 0)
	{
		ECanCreateConnectionResponse ReplyBreakOutputs;
		if (InputPin == PinA)
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_A;
		}
		else
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_B;
		}
		return FPinConnectionResponse(ReplyBreakOutputs, VOXEL_LOCTEXT("Replace existing connections"));
	}
	
	if (OutputCategory == EVoxelPinCategory::Exec && OutputPin->LinkedTo.Num() > 0)
	{
		ECanCreateConnectionResponse ReplyBreakOutputs;
		if (OutputPin == PinA)
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_A;
		}
		else
		{
			ReplyBreakOutputs = CONNECT_RESPONSE_BREAK_OTHERS_B;
		}
		return FPinConnectionResponse(ReplyBreakOutputs, VOXEL_LOCTEXT("Replace existing connections"));
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, TEXT(""));
}

void UVoxelGraphSchema::GetAllVoxelNodeActions(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph) const
{
	InitVoxelNodeClasses();

	auto* FromPin = ActionMenuBuilder.FromPin;
	EVoxelPinCategory Category = FromPin ? FVoxelPinCategory::FromString(FromPin->PinType.PinCategory) : EVoxelPinCategory::Wildcard;

	const int32 RerouteNodePriority = 20;
	const int32 LocalVariablesPriority = 10;
	const int32 SetterNodesPriority = 5;
	const int32 ParameterNodesPriority = 0;
	const int32 MacroNodesPriority = 0;

	const auto PinMatchesNode = [&](UVoxelNode* Node)
	{
		if (Category == EVoxelPinCategory::Vector)
		{
			// Make sure to check the opposite direction of FromPin
			return UVoxelGraphNode::HasVectorPin(*Node, FromPin->Direction == EGPD_Input ? EGPD_Output : EGPD_Input);
		}
		
		// Make sure to check the opposite direction of FromPin
		return FromPin->Direction == EGPD_Input
			? Node->HasOutputPinWithCategory(Category)
			: Node->HasInputPinWithCategory(Category);
	};

	// Macros
	{
		// Load the asset registry module
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

		// Collect a full list of assets with the specified class
		TArray<FAssetData> AssetDataList;
		AssetRegistryModule.Get().GetAssetsByClass(UVoxelGraphMacro::StaticClass()->GetClassPathName(), AssetDataList);

		for (const FAssetData& AssetData : AssetDataList)
		{
			FStreamableManager AssetLoader;
			UVoxelGraphMacro* Macro = Cast<UVoxelGraphMacro>(AssetLoader.LoadSynchronous(AssetData.GetSoftObjectPath()));

			if (!Macro || !Macro->InputNode || !Macro->OutputNode || !Macro->bShowInContextMenu)
			{
				continue;
			}

			const auto PinMatchesMacro = [&]()
			{
				// Make sure to check the opposite direction of FromPin
				auto* Node = FromPin->Direction == EGPD_Input ? static_cast<UVoxelNode*>(Macro->OutputNode) : Macro->InputNode;
				if (!Node)
				{
					return false;
				}

				if (Category != EVoxelPinCategory::Vector && Macro->bVectorOnlyNode)
				{
					// Having all the vector macros when dragging a float is annoying
					return false;
				}

				return PinMatchesNode(Node);
			};
			
			if (Macro->bShowInContextMenu && (!FromPin || PinMatchesMacro()))
			{
				const FText Name = Macro->GetMacroName();
				const FText AddToolTip = FText::FromString(Macro->Tooltip);
				const FText Keywords = FText::FromString(Macro->Keywords);
				const FText MacroCategory = Macro->GetMacroCategory();

				TSharedPtr<FVoxelGraphSchemaAction_NewMacroNode> NewNodeAction(
					new FVoxelGraphSchemaAction_NewMacroNode(
						MacroCategory,
						Name,
						AddToolTip,
						MacroNodesPriority,
						Keywords));
				NewNodeAction->Macro = Macro;
				ActionMenuBuilder.AddAction(NewNodeAction);
			}
		}
	}

	// Local variables declaration
	{
		TSharedPtr<FVoxelGraphSchemaAction_NewLocalVariableDeclaration> NewNodeAction(
			new FVoxelGraphSchemaAction_NewLocalVariableDeclaration(
				FText::GetEmpty(),
				VOXEL_LOCTEXT("Create local variable"),
				VOXEL_LOCTEXT("Create a new local variable here"),
				RerouteNodePriority));
		NewNodeAction->PinCategory = EVoxelPinCategory::Float;
		bool bAdd = false;
		if (FromPin)
		{
			if (FromPin->Direction == EGPD_Output)
			{
				if (Category != EVoxelPinCategory::Exec && 
					Category != EVoxelPinCategory::Wildcard && 
					Category != EVoxelPinCategory::Vector)
				{
					NewNodeAction->DefaultName = FromPin->PinName;
					NewNodeAction->PinCategory = Category;
					bAdd = true;
				}
			}
		}
		else
		{
			bAdd = true;
		}
		if (bAdd)
		{
			ActionMenuBuilder.AddAction(NewNodeAction);
		}
	}

	// For the palette actions CurrentGraph is null
	if (CurrentGraph)
	{
		auto* Graph = CastChecked<UVoxelEdGraph>(CurrentGraph);
		auto* Generator = Graph->GetGenerator();

		// Local variables usage
		if (!FromPin || FromPin->Direction == EGPD_Input)
		{
			for (auto& Node : Generator->AllNodes)
			{
				auto* Declaration = Cast<UVoxelLocalVariableDeclaration>(Node);
				if (Declaration && (!FromPin || Declaration->GetCategory() == Category))
				{
					const FText Name = FText::FromName(Declaration->Name);
					const FText AddToolTip = FText::Format(VOXEL_LOCTEXT("Use {0} here"), Name);

					TSharedPtr<FVoxelGraphSchemaAction_NewLocalVariableUsage> NewNodeAction(
						new FVoxelGraphSchemaAction_NewLocalVariableUsage(
							VOXEL_LOCTEXT("Local variables"),
							Name,
							AddToolTip,
							LocalVariablesPriority));
					NewNodeAction->Declaration = Declaration;
					ActionMenuBuilder.AddAction(NewNodeAction);
				}
			}
		}

		// Setter nodes
		{
			auto Outputs = Generator->GetOutputs();
			for (auto It : Outputs)
			{
				auto Output = It.Value;
				auto Index = It.Key;
				if (FVoxelGraphOutputsUtils::IsVoxelGraphOutputHidden(Index))
				{
					continue;
				}
				if (!FromPin || Category == EVoxelPinCategory::Exec || Category == FVoxelPinCategory::DataPinToPin(Output.Category))
				{
					const FText Name = FText::FromString("Set " + Output.Name.ToString());
					const FText AddToolTip = FText::Format(VOXEL_LOCTEXT("Adds {0} node here"), Name);

					TSharedPtr<FVoxelGraphSchemaAction_NewSetterNode> NewNodeAction(
						new FVoxelGraphSchemaAction_NewSetterNode(
							VOXEL_LOCTEXT("Setter nodes"),
							Name,
							AddToolTip,
							SetterNodesPriority));
					NewNodeAction->Index = Index;
					ActionMenuBuilder.AddAction(NewNodeAction);
				}
			}
		}
	}

	for (auto& NodeClass : VoxelNodeClasses)
	{
		UVoxelNode* DefaultNode = NodeClass->GetDefaultObject<UVoxelNode>();
		if (!FromPin || PinMatchesNode(DefaultNode))
		{
			const auto GetCategory = [](UClass* Class)
			{
				return Class->GetMetaDataText(TEXT("Category"), TEXT("UObjectCategory"), Class->GetFullGroupName(false));
			};

			FText ActionCategory = GetCategory(NodeClass);
			if (ActionCategory.IsEmpty())
			{
				UClass* Class = NodeClass->GetSuperClass();
				while (Class && ActionCategory.IsEmpty())
				{
					ActionCategory = GetCategory(Class);
					Class = Class->GetSuperClass();
				}
			}
			
			int32 Priority = 0;

			if (NodeClass->IsChildOf(UVoxelExposedNode::StaticClass()))
			{
				Priority = ParameterNodesPriority;
			}
			if (NodeClass->IsChildOf(UVoxelSetterNode::StaticClass()))
			{
				Priority = SetterNodesPriority;
			}

			FText Name = FText::FromString(NodeClass->GetDescription());
			FText AddToolTip = NodeClass->GetToolTipText();
			FText Keywords = NodeClass->GetMetaDataText(TEXT("Keywords"), TEXT("UObjectKeywords"), GetClass()->GetFullGroupName(false));

			TSharedPtr<FVoxelGraphSchemaAction_NewNode> NewNodeAction(
				new FVoxelGraphSchemaAction_NewNode(
					ActionCategory,
					Name,
					AddToolTip,
					Priority,
					Keywords));
			NewNodeAction->VoxelNodeClass = NodeClass;
			ActionMenuBuilder.AddAction(NewNodeAction);
		}
	}

	if (FromPin)
	{
		const FText MenuDescription = VOXEL_LOCTEXT("Add reroute node");
		const FText ToolTip = VOXEL_LOCTEXT("Create a reroute node.");
		TSharedPtr<FVoxelGraphSchemaAction_NewKnotNode> NewNodeAction(new FVoxelGraphSchemaAction_NewKnotNode(FText::GetEmpty(), MenuDescription, ToolTip, RerouteNodePriority));
		ActionMenuBuilder.AddAction(NewNodeAction);
	}
}

void UVoxelGraphSchema::GetCommentAction(FGraphActionMenuBuilder& ActionMenuBuilder, const UEdGraph* CurrentGraph /*= NULL*/) const
{
	if (!ActionMenuBuilder.FromPin)
	{
		const bool bIsManyNodesSelected = CurrentGraph ? (FVoxelGraphEditorUtilities::GetNumberOfSelectedNodes(CurrentGraph) > 0) : false;
		const FText MenuDescription = bIsManyNodesSelected ? VOXEL_LOCTEXT("Create Comment from Selection") : VOXEL_LOCTEXT("Add Comment");
		const FText ToolTip = VOXEL_LOCTEXT("Creates a comment.");

		TSharedPtr<FVoxelGraphSchemaAction_NewComment> NewAction(new FVoxelGraphSchemaAction_NewComment(FText::GetEmpty(), MenuDescription, ToolTip, 0));
		ActionMenuBuilder.AddAction(NewAction);
	}

}

void UVoxelGraphSchema::InitVoxelNodeClasses()
{
	if (bVoxelNodeClassesInitialized)
	{
		return;
	}
	VoxelNodeClasses.Empty();

	// Construct list of non-abstract voxel node classes.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(UVoxelNode::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NotPlaceable | CLASS_Deprecated))
		{
			VoxelNodeClasses.Add(*It);
		}
	}

	VoxelNodeClasses.Sort();

	bVoxelNodeClassesInitialized = true;
}