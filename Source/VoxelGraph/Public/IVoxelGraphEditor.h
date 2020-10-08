// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

enum class EVoxelGraphNodeMessageType : int32;
struct FVoxelGraphMessage;
class FVoxelCompilationNode;
class UEdGraph;
class UEdGraphNode;
class UVoxelNode;
class UVoxelGraphGenerator;

#if WITH_EDITOR
enum class EVoxelGraphPreviewFlags
{
	None = 0,
	ManualPreview          = 1 << 0,
	UpdateMeshSettings     = 1 << 1,
	UpdateTextures         = 1 << 2,
	UpdatePlaceableItems   = 1 << 3,
	UpdateAll = UpdateMeshSettings | UpdateTextures | UpdatePlaceableItems,
};
ENUM_CLASS_FLAGS(EVoxelGraphPreviewFlags);

/** 
 * Interface for voxel graph interaction with the VoxelEditor module.
 */
class VOXELGRAPH_API IVoxelGraphEditor
{
public:
	virtual ~IVoxelGraphEditor() = default;

	// Called when creating a new voxel graph.
	virtual UEdGraph* CreateNewVoxelGraph(UVoxelGraphGenerator* InGenerator) = 0;

	// Sets up a voxel node.
	virtual void CreateVoxelGraphNode(UEdGraph* VoxelGraph, UVoxelNode* VoxelNode, bool bSelectNewNode) = 0;

	// Compiles voxel nodes from graph nodes.
	virtual void CompileVoxelNodesFromGraphNodes(UVoxelGraphGenerator* Generator) = 0;

	virtual void UpdatePreview(UVoxelGraphGenerator* Generator, EVoxelGraphPreviewFlags Flags) = 0;

	virtual void SelectNodesAndZoomToFit(UEdGraph* Graph, const TArray<UEdGraphNode*>& Nodes) = 0;
	virtual void RefreshNodesMessages(UEdGraph* Graph) = 0;

	virtual void DebugNodes(UEdGraph* DebugGraph, const TSet<FVoxelCompilationNode*>& Nodes) = 0;

	virtual void AddMessages(const UVoxelGraphGenerator* Generator, const TArray<FVoxelGraphMessage>& Messages) = 0;
	virtual void ClearMessages(const UVoxelGraphGenerator* Generator, bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear) = 0;
	
public:
	// Sets the voxel graph editor implementation
	static void SetVoxelGraphEditor(TSharedPtr<IVoxelGraphEditor> InVoxelGraphEditor);

	inline static IVoxelGraphEditor* GetVoxelGraphEditor() { return VoxelGraphEditor.Get(); }

private:
	// Ptr to interface to voxel editor operations.
	static TSharedPtr<IVoxelGraphEditor> VoxelGraphEditor;
};
#endif