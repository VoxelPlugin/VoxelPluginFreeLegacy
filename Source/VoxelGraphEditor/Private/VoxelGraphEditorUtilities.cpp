// Copyright 2020 Phyronnaz

#include "VoxelGraphEditorUtilities.h"
#include "IVoxelGraphEditorToolkit.h"
#include "VoxelGraphGenerator.h"
#include "VoxelEdGraph.h"

#include "EdGraph/EdGraph.h"
#include "Toolkits/ToolkitManager.h"

bool FVoxelGraphEditorUtilities::CanPasteNodes(const UEdGraph* Graph)
{
	bool bCanPaste = false;
	TSharedPtr<IVoxelGraphEditorToolkit> VoxelEditor = GetIVoxelEditorForGraph(Graph);
	if (VoxelEditor.IsValid())
	{
		bCanPaste = VoxelEditor->CanPasteNodes();
	}
	return bCanPaste;
}

void FVoxelGraphEditorUtilities::PasteNodesHere(UEdGraph* Graph, const FVector2D& Location)
{
	TSharedPtr<IVoxelGraphEditorToolkit> VoxelEditor = GetIVoxelEditorForGraph(Graph);
	if (VoxelEditor.IsValid())
	{
		VoxelEditor->PasteNodesHere(Location);
	}
}

bool FVoxelGraphEditorUtilities::GetBoundsForSelectedNodes(const UEdGraph* Graph, FSlateRect& Rect, float Padding)
{
	TSharedPtr<IVoxelGraphEditorToolkit> VoxelEditor = GetIVoxelEditorForGraph(Graph);
	if (VoxelEditor.IsValid())
	{
		return VoxelEditor->GetBoundsForSelectedNodes(Rect, Padding);
	}
	return false;
}

int32 FVoxelGraphEditorUtilities::GetNumberOfSelectedNodes(const UEdGraph* Graph)
{
	TSharedPtr<IVoxelGraphEditorToolkit> VoxelEditor = GetIVoxelEditorForGraph(Graph);
	if (VoxelEditor.IsValid())
	{
		return VoxelEditor->GetNumberOfSelectedNodes();
	}
	return 0;
}

TSet<UObject*> FVoxelGraphEditorUtilities::GetSelectedNodes(const UEdGraph* Graph)
{
	TSharedPtr<IVoxelGraphEditorToolkit> VoxelEditor = GetIVoxelEditorForGraph(Graph);
	if (VoxelEditor.IsValid())
	{
		return VoxelEditor->GetSelectedNodes();
	}
	return TSet<UObject*>();
}

TSharedPtr<IVoxelGraphEditorToolkit> FVoxelGraphEditorUtilities::GetIVoxelEditorForGraph(const UObject* ObjectToFocusOn)
{
	if (!ensure(ObjectToFocusOn)) return {};

	// Find the associated VoxelGraphGenerator
	UVoxelGraphGenerator* VoxelGraphGenerator = Cast<const UVoxelEdGraph>(ObjectToFocusOn)->GetGenerator();

	TSharedPtr<IVoxelGraphEditorToolkit> VoxelEditor;
	if (VoxelGraphGenerator)
	{
		TSharedPtr<IToolkit> FoundAssetEditor = FToolkitManager::Get().FindEditorForAsset(VoxelGraphGenerator);
		if (FoundAssetEditor.IsValid())
		{
			VoxelEditor = StaticCastSharedPtr<IVoxelGraphEditorToolkit>(FoundAssetEditor);
		}
	}
	return VoxelEditor;
}