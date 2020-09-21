// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNoiseNodes.h"
#include "VoxelNoiseNodesMacros.h"
#include "VoxelGavoronoiNoiseNode.generated.h"

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_GavoronoiNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()

public:
	UVoxelNode_GavoronoiNoise();
	
	UPROPERTY(EditAnywhere, Category = "Gavoronoi Noise settings", meta = (UIMin = 0, UIMax = 0.5))
	float Jitter = 0.45;
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_GavoronoiNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()

public:
	UVoxelNode_GavoronoiNoiseFractal();
	
	UPROPERTY(EditAnywhere, Category = "Gavoronoi Noise settings", meta = (UIMin = 0, UIMax = 0.5))
	float Jitter = 0.45;
};

// 2D Gavoronoi Noise
UCLASS(DisplayName = "2D Gavoronoi Noise", Category = "Noise|Gavoronoi Noise")
class VOXELGRAPH_API UVoxelNode_2DGavoronoiNoise : public UVoxelNode_GavoronoiNoise
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	virtual uint32 GetDimension() const override { return 2; }
};

// 2D Gavoronoi Noise Fractal
UCLASS(DisplayName = "2D Gavoronoi Noise Fractal", Category = "Noise|Gavoronoi Noise")
class VOXELGRAPH_API UVoxelNode_2DGavoronoiNoiseFractal : public UVoxelNode_GavoronoiNoiseFractal
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	virtual uint32 GetDimension() const override { return 2; }
};