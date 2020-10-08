// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGraphAssetNodes.generated.h"

UCLASS(Abstract, Category = "Graph Asset")
class VOXELGRAPH_API UVoxelGraphAssetNode : public UVoxelNodeWithContext
{
	GENERATED_BODY()

public:
	UVoxelGraphAssetNode() = default;

	// Generator to sample from when not used as an asset. Useful to preview. Not used when compiled to C++
	UPROPERTY(EditAnywhere, Category = "Preview", meta = (ReconstructNode))
	FVoxelGeneratorPicker DefaultGenerator;

	//~ Begin UVoxelNode Interface
	virtual int32 GetMaxInputPins() const override;
	//~ End UVoxelNode Interface
};

// Get the previous generator value. Only for graph assets
UCLASS(DisplayName= "Get Previous Generator Value")
class VOXELGRAPH_API UVoxelNode_EditGetValue : public UVoxelGraphAssetNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_EditGetValue();
};

// Get the previous generator material. Only for graph assets
UCLASS(DisplayName= "Get Previous Generator Material")
class VOXELGRAPH_API UVoxelNode_EditGetMaterial : public UVoxelGraphAssetNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UVoxelNode_EditGetMaterial();
};

// Get the previous generator custom output. Only for graph assets
UCLASS(DisplayName= "Get Previous Generator Custom Output")
class VOXELGRAPH_API UVoxelNode_EditGetCustomOutput : public UVoxelGraphAssetNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UVoxelNode_EditGetCustomOutput();

	UPROPERTY(EditAnywhere, Category = "Config")
	FName OutputName = "Value";

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface
};

// Get the material hardness
UCLASS(DisplayName= "Get Hardness", Category = "Material")
class VOXELGRAPH_API UVoxelNode_EditGetHardness : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UVoxelNode_EditGetHardness();
};