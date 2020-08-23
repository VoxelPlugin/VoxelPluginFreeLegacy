// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelVoronoiNoiseNodes.generated.h"

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_VoronoiNoiseBase : public UVoxelNodeHelper
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voronoi settings", meta = (ReconstructNode))
	bool bComputeNeighbors = false;

	UPROPERTY()
	int32 Dimension;

	//~ Begin UVoxelNode Interface
	virtual int32 GetMinInputPins() const override;
	virtual int32 GetMaxInputPins() const override;
	virtual int32 GetOutputPinsCount() const override;
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;
	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;
	virtual FString GetInputPinToolTip(int32 PinIndex) const override;
	virtual FString GetOutputPinToolTip(int32 PinIndex) const override;
	virtual FVoxelPinDefaultValueBounds GetInputPinDefaultValueBounds(int32 PinIndex) const override;
	virtual FString GetInputPinDefaultValue(int32 PinIndex) const override;
	//~ End UVoxelNode Interface

private:
	const FVoxelPinsHelper& GetPins() const;
};

// 2D Voronoi Noise
UCLASS(DisplayName = "2D Voronoi Noise", Category = "Noise|Voronoi Noise")
class VOXELGRAPH_API UVoxelNode_2DVoronoiNoise : public UVoxelNode_VoronoiNoiseBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_2DVoronoiNoise();
};