// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelRandomNodes.generated.h"

// A random number >= Min and <= Max
UCLASS(DisplayName = "Rand Float", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_RandomFloat : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float Min = 0;
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float Max = 1;

	UVoxelNode_RandomFloat();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface
};

// A random number >= Min and <= Max
UCLASS(DisplayName = "Rand int", Category = "Math|Integer")
class VOXELGRAPH_API UVoxelNode_RandomInt : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 Min = 0;
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 Max = 100;

	UVoxelNode_RandomInt();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface
};