// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IVoxelGraphEditor.h"

class FVoxelGraphEditor : public IVoxelGraphEditor
{
public:
	virtual UEdGraph* CreateNewVoxelGraph(UVoxelGraphGenerator* InVoxelWorldGenerator) override;
	virtual void CreateVoxelGraphNode(UEdGraph* VoxelGraph, UVoxelNode* InVoxelNode, bool bSelectNewNode) override;
	virtual void CompileVoxelNodesFromGraphNodes(UVoxelGraphGenerator* WorldGenerator) override;
	virtual void UpdatePreview(UVoxelGraphGenerator* WorldGenerator, bool bForce, bool bUpdateTextures) override;
	virtual void SelectNodesAndZoomToFit(UEdGraph* Graph, const TArray<UEdGraphNode*>& Nodes) override;
	virtual void RefreshNodesMessages(UEdGraph* Graph) override;
	virtual void DebugNodes(UEdGraph* DebugGraph, const TSet<FVoxelCompilationNode*>& Nodes) override;
	virtual void AddMessages(const UVoxelGraphGenerator* WorldGenerator, const TArray<FVoxelGraphMessage>& Messages) override;
	virtual void ClearMessages(const UVoxelGraphGenerator* WorldGenerator, bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear) override;
};