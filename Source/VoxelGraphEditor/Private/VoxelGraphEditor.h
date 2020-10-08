// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IVoxelGraphEditor.h"

class FVoxelGraphEditor : public IVoxelGraphEditor
{
public:
	virtual UEdGraph* CreateNewVoxelGraph(UVoxelGraphGenerator* InGenerator) override;
	virtual void CreateVoxelGraphNode(UEdGraph* VoxelGraph, UVoxelNode* InVoxelNode, bool bSelectNewNode) override;
	virtual void CompileVoxelNodesFromGraphNodes(UVoxelGraphGenerator* Generator) override;
	virtual void UpdatePreview(UVoxelGraphGenerator* Generator, EVoxelGraphPreviewFlags Flags) override;
	virtual void SelectNodesAndZoomToFit(UEdGraph* Graph, const TArray<UEdGraphNode*>& Nodes) override;
	virtual void RefreshNodesMessages(UEdGraph* Graph) override;
	virtual void DebugNodes(UEdGraph* DebugGraph, const TSet<FVoxelCompilationNode*>& Nodes) override;
	virtual void AddMessages(const UVoxelGraphGenerator* Generator, const TArray<FVoxelGraphMessage>& Messages) override;
	virtual void ClearMessages(const UVoxelGraphGenerator* Generator, bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear) override;
};