// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class SGraphNodeDefaultVoxel : public SGraphNode
{
public:

	SLATE_BEGIN_ARGS(SGraphNodeDefaultVoxel)
		: _GraphNodeObj(static_cast<UEdGraphNode*>(NULL))
	{
	}

	SLATE_ARGUMENT(UEdGraphNode*, GraphNodeObj)
	SLATE_END_ARGS()


	void Construct(const FArguments& InArgs)
	{
		GraphNode = InArgs._GraphNodeObj;
		SetCursor(EMouseCursor::CardinalCross);
		UpdateGraphNode();
	}
};