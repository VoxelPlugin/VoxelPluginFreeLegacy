// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class IVoxelGraphEditorToolkit;
class UEdGraph;
class FSlateRect;

namespace FVoxelGraphEditorUtilities
{
	/** Can we paste to this graph? */
	bool CanPasteNodes(const UEdGraph* Graph);

	/** Perform paste on graph, at location */
	void  PasteNodesHere(UEdGraph* Graph, const FVector2D& Location);

	/** Get the bounding area for the currently selected nodes
	 *
	 * @param Graph The Graph we are finding bounds for
	 * @param Rect Final output bounding area, including padding
	 * @param Padding An amount of padding to add to all sides of the bounds
	 *
	 * @return false if nothing is selected*/
	bool GetBoundsForSelectedNodes(const UEdGraph* Graph, FSlateRect& Rect, float Padding = 0.0f);

	/** Gets the number of nodes that are currently selected */
	int32 GetNumberOfSelectedNodes(const UEdGraph* Graph);

	/** Get the currently selected set of nodes */
	TSet<UObject*> GetSelectedNodes(const UEdGraph* Graph);

	/** Get IVoxelEditor for given object, if it exists */
	TSharedPtr<IVoxelGraphEditorToolkit> GetIVoxelEditorForGraph(const UObject* ObjectToFocusOn);
}