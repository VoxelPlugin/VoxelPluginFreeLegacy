// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelIfNode.generated.h"

UENUM()
enum class EVoxelNodeIfBranchToUseForRangeAnalysis : uint8
{
	None,
	UseTrue,
	UseFalse
};

// Branch node
UCLASS(DisplayName = "If", Category = "Flow Control", meta = (Keywords = "branch"))
class VOXELGRAPH_API UVoxelNode_If : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	// Use this to hide range analysis errors on this node
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bIgnoreRangeAnalysisErrors = false;
	
	// If the condition range analysis fails, use this branch instead of failing.
	// DO NOT CHANGE THIS UNLESS YOU KNOW WHAT YOU ARE DOING, ELSE YOUR WORLD WILL HAVE HOLES
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Voxel")
	EVoxelNodeIfBranchToUseForRangeAnalysis BranchToUseForRangeAnalysis = EVoxelNodeIfBranchToUseForRangeAnalysis::None;

public:
	UVoxelNode_If();
	
};

// Helper
UCLASS(NotPlaceable)
class VOXELGRAPH_API UVoxelNode_IfWithDefaultToFalse : public UVoxelNode_If
{
	GENERATED_BODY()

public:
	UVoxelNode_IfWithDefaultToFalse()
	{
		BranchToUseForRangeAnalysis = EVoxelNodeIfBranchToUseForRangeAnalysis::UseFalse;
	}
};

// Helper
UCLASS(NotPlaceable)
class VOXELGRAPH_API UVoxelNode_IfWithDefaultToTrue : public UVoxelNode_If
{
	GENERATED_BODY()

public:
	UVoxelNode_IfWithDefaultToTrue()
	{
		BranchToUseForRangeAnalysis = EVoxelNodeIfBranchToUseForRangeAnalysis::UseTrue;
	}
};

