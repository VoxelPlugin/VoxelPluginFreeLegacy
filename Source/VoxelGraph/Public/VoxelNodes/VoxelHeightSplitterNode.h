// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodes/VoxelNodeHelperMacros.h"
#include "VoxelHeightSplitterNode.generated.h"

// Splits a float input based on different layers, and outputs the strength of each layer
// Input heights must be ordered! 
UCLASS(DisplayName = "Height Splitter", Category = "Math|Float")
class VOXELGRAPH_API UVoxelNode_HeightSplitter : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ReconstructNode), meta = (ClampMin = 1, ClampMax = 30))
	int32 NumSplits = 4;

	UVoxelNode_HeightSplitter();

	//~ Begin UVoxelNode Interface
	virtual int32 GetMinInputPins() const override;
	virtual int32 GetMaxInputPins() const override;
	virtual int32 GetOutputPinsCount() const override;

	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;

	virtual FString GetInputPinDefaultValue(int32 PinIndex) const override;
	//~ End UVoxelNode Interface
};