// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "EdGraphUtilities.h"
#include "SGraphNodeKnot.h"
#include "VoxelGraphNodes/SVoxelGraphNode.h"
#include "VoxelGraphNodes/VoxelGraphNode_Knot.h"
#include "VoxelGraphNodes/VoxelGraphNode.h"
#include "VoxelNodes/VoxelParameterNodes.h"
#include "VoxelNodes/VoxelAssetPickerNode.h"

class FVoxelGraphNodeFactory : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(class UEdGraphNode* InNode) const override
	{
		if (auto* Knot = Cast<UVoxelGraphNode_Knot>(InNode))
		{
			return SNew(SGraphNodeKnot, Knot);
		}
		else if (auto* VoxelGraphNodeBase = Cast<UVoxelGraphNode_Base>(InNode))
		{
			if (auto* VoxelGraphNode = Cast<UVoxelGraphNode>(InNode))
			{
				if (auto* ColorNode = Cast<UVoxelNode_ColorParameter>(VoxelGraphNode->VoxelNode))
				{
					return SNew(SVoxelColorGraphNode, VoxelGraphNode, ColorNode);
				}
				if (auto* AssetPickerNode = Cast<UVoxelAssetPickerNode>(VoxelGraphNode->VoxelNode))
				{
					return SNew(SVoxelAssetPickerGraphNode, VoxelGraphNode, AssetPickerNode);
				}
			}
			return SNew(SVoxelGraphNode, VoxelGraphNodeBase);
		}
		return nullptr;
	}
};