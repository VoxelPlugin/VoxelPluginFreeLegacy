// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/AssetEditorToolkit.h"

class FAdvancedPreviewScene;
class FVoxelGraphCompiler;
class UVoxelGraphGenerator;
class UEdGraphNode;
class FVoxelCompilationNode;

struct FVoxelGraphMessage;

enum class EVoxelGraphPreviewFlags;
enum class EVoxelGraphNodeMessageType : int32;

class IVoxelGraphEditorToolkit : public FAssetEditorToolkit
{
public:
	// Checks whether nodes can currently be pasted
	virtual bool CanPasteNodes() const = 0;
	// Paste nodes at a specific location
	virtual void PasteNodesHere(const FVector2D& Location) = 0;

	// Get the bounding area for the currently selected nodes. returns false if nothing is selected
	virtual bool GetBoundsForSelectedNodes(class FSlateRect& Rect, float Padding) = 0;

	// Gets the number of nodes that are currently selected
	virtual int32 GetNumberOfSelectedNodes() const = 0;

	// Get the currently selected set of nodes
	virtual TSet<UObject*> GetSelectedNodes() const = 0;

	virtual void SelectNodesAndZoomToFit(const TArray<UEdGraphNode*>& Nodes) = 0;

	virtual void RefreshNodesMessages() = 0;

	virtual void TriggerUpdatePreview(EVoxelGraphPreviewFlags Flags) = 0;

	virtual FAdvancedPreviewScene* GetPreviewScene() const = 0;

	virtual void DebugNodes(const TSet<FVoxelCompilationNode*>& Nodes) = 0;
	
	virtual void AddMessages(const TArray<FVoxelGraphMessage>& Messages) = 0;
	virtual void ClearMessages(bool bClearAll, EVoxelGraphNodeMessageType MessagesToClear) = 0;
};