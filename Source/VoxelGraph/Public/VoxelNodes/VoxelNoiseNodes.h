// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNoiseNodesBase.h"
#include "VoxelNoiseNodes.generated.h"

#define GENERATED_NOISENODE_BODY_IMPL(Dimension, FunctionName, Body, Parent) \
	virtual uint32 GetDimension() const override final { return Dimension; }

#define GENERATED_NOISENODE_BODY_DIM_IMPL(Dimension, Derivative, Fractal, FunctionName, Parent) \
	virtual uint32 GetDimension() const override final { return Dimension; }

#define GENERATED_NOISENODE_BODY_DIM2(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(2,,, FunctionName, FVoxelNoiseComputeNode)
#define GENERATED_NOISENODE_BODY_DIM3(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(3,,, FunctionName, FVoxelNoiseComputeNode)

#define GENERATED_NOISENODE_BODY_FRACTAL_DIM2(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(2,, _FRACTAL, FunctionName, FVoxelNoiseFractalComputeNode)
#define GENERATED_NOISENODE_BODY_FRACTAL_DIM3(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(3,, _FRACTAL, FunctionName, FVoxelNoiseFractalComputeNode)

#define GENERATED_NOISENODE_BODY_DERIVATIVE_DIM2(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(2, _DERIVATIVE,, FunctionName, FVoxelNoiseComputeNode)
#define GENERATED_NOISENODE_BODY_DERIVATIVE_DIM3(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(3, _DERIVATIVE,, FunctionName, FVoxelNoiseComputeNode)

#define GENERATED_NOISENODE_BODY_FRACTAL_DERIVATIVE_DIM2(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(2, _DERIVATIVE, _FRACTAL, FunctionName, FVoxelNoiseFractalComputeNode)
#define GENERATED_NOISENODE_BODY_FRACTAL_DERIVATIVE_DIM3(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(3, _DERIVATIVE, _FRACTAL, FunctionName, FVoxelNoiseFractalComputeNode)

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// 2D Value Noise
UCLASS(DisplayName = "2D Value Noise", Category = "Noise|Value Noise")
class VOXELGRAPH_API UVoxelNode_2DValueNoise : public UVoxelNode_NoiseNodeWithDerivative
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DERIVATIVE_DIM2(GetValue)
};

// 2D Value Noise Fractal
UCLASS(DisplayName = "2D Value Noise Fractal", Category = "Noise|Value Noise")
class VOXELGRAPH_API UVoxelNode_2DValueNoiseFractal : public UVoxelNode_NoiseNodeWithDerivativeFractal
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_FRACTAL_DERIVATIVE_DIM2(GetValueFractal)
};

//////////////////////////////////////////////////////////////////////////////////////

// 2D Perlin Noise
UCLASS(DisplayName = "2D Perlin Noise", Category = "Noise|Perlin Noise")
class VOXELGRAPH_API UVoxelNode_2DPerlinNoise : public UVoxelNode_NoiseNodeWithDerivative
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DERIVATIVE_DIM2(GetPerlin)
};

// 2D Perlin Noise Fractal
UCLASS(DisplayName = "2D Perlin Noise Fractal", Category = "Noise|Perlin Noise")
class VOXELGRAPH_API UVoxelNode_2DPerlinNoiseFractal : public UVoxelNode_NoiseNodeWithDerivativeFractal
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_FRACTAL_DERIVATIVE_DIM2(GetPerlinFractal)
};

//////////////////////////////////////////////////////////////////////////////////////

// 2D Simplex Noise
UCLASS(DisplayName = "2D Simplex Noise", Category = "Noise|Simplex Noise")
class VOXELGRAPH_API UVoxelNode_2DSimplexNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DIM2(GetSimplex)
};

// 2D Simplex Noise Fractal
UCLASS(DisplayName = "2D Simplex Noise Fractal", Category = "Noise|Simplex Noise")
class VOXELGRAPH_API UVoxelNode_2DSimplexNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_FRACTAL_DIM2(GetSimplexFractal)
};

//////////////////////////////////////////////////////////////////////////////////////

// 2D Cubic Noise
UCLASS(DisplayName = "2D Cubic Noise", Category = "Noise|Cubic Noise")
class VOXELGRAPH_API UVoxelNode_2DCubicNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DIM2(GetCubic)
};

// 2D Cubic Noise Fractal
UCLASS(DisplayName = "2D Cubic Noise Fractal", Category = "Noise|Cubic Noise")
class VOXELGRAPH_API UVoxelNode_2DCubicNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_FRACTAL_DIM2(GetCubicFractal)
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// 3D Value Noise
UCLASS(DisplayName = "3D Value Noise", Category = "Noise|Value Noise")
class VOXELGRAPH_API UVoxelNode_3DValueNoise : public UVoxelNode_NoiseNodeWithDerivative
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DERIVATIVE_DIM3(GetValue)
};

// 3D Value Noise Fractal
UCLASS(DisplayName = "3D Value Noise Fractal", Category = "Noise|Value Noise")
class VOXELGRAPH_API UVoxelNode_3DValueNoiseFractal : public UVoxelNode_NoiseNodeWithDerivativeFractal
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_FRACTAL_DERIVATIVE_DIM3(GetValueFractal)
};

//////////////////////////////////////////////////////////////////////////////////////

// 3D Perlin Noise
UCLASS(DisplayName = "3D Perlin Noise", Category = "Noise|Perlin Noise")
class VOXELGRAPH_API UVoxelNode_3DPerlinNoise : public UVoxelNode_NoiseNodeWithDerivative
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DERIVATIVE_DIM3(GetPerlin)
};

// 3D Perlin Noise Fractal
UCLASS(DisplayName = "3D Perlin Noise Fractal", Category = "Noise|Perlin Noise")
class VOXELGRAPH_API UVoxelNode_3DPerlinNoiseFractal : public UVoxelNode_NoiseNodeWithDerivativeFractal
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_FRACTAL_DERIVATIVE_DIM3(GetPerlinFractal)
};

//////////////////////////////////////////////////////////////////////////////////////

// 3D Simplex Noise
UCLASS(DisplayName = "3D Simplex Noise", Category = "Noise|Simplex Noise")
class VOXELGRAPH_API UVoxelNode_3DSimplexNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DIM3(GetSimplex)
};

// 3D Simplex Noise Fractal
UCLASS(DisplayName = "3D Simplex Noise Fractal", Category = "Noise|Simplex Noise")
class VOXELGRAPH_API UVoxelNode_3DSimplexNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_FRACTAL_DIM3(GetSimplexFractal)
};

//////////////////////////////////////////////////////////////////////////////////////

// 3D Cubic Noise
UCLASS(DisplayName = "3D Cubic Noise", Category = "Noise|Cubic Noise")
class VOXELGRAPH_API UVoxelNode_3DCubicNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DIM3(GetCubic)
};

// 3D Cubic Noise Fractal
UCLASS(DisplayName = "3D Cubic Noise Fractal", Category = "Noise|Cubic Noise")
class VOXELGRAPH_API UVoxelNode_3DCubicNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_FRACTAL_DIM3(GetCubicFractal)
};

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_CellularNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Cellular Noise settings")
	ECellularDistanceFunction DistanceFunction;

	UPROPERTY(EditAnywhere, Category = "Cellular Noise settings")
	ECellularReturnType ReturnType;

	UPROPERTY(EditAnywhere, Category = "Cellular Noise settings")
	float Jitter = 0.45;
};


// 2D Cellular Noise
UCLASS(DisplayName = "2D Cellular Noise", Category = "Noise|Cellular Noise")
class VOXELGRAPH_API UVoxelNode_2DCellularNoise : public UVoxelNode_CellularNoise
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DIM_IMPL(2,,, GetCellular, FVoxelCellularNoiseComputeNode)
};

// 3D Cellular Noise
UCLASS(DisplayName = "3D Cellular Noise", Category = "Noise|Cellular Noise")
class VOXELGRAPH_API UVoxelNode_3DCellularNoise : public UVoxelNode_CellularNoise
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DIM_IMPL(3,,, GetCellular, FVoxelCellularNoiseComputeNode)
};

// 2D IQ Noise: uses the derivative to "smooth" the fractals. 
// For more details: 
// http://www.iquilezles.org/www/articles/morenoise/morenoise.htm
// http://www.decarpentier.nl/scape-procedural-basics
UCLASS(DisplayName = "2D IQ Noise", Category = "Noise|IQ Noise")
class VOXELGRAPH_API UVoxelNode_2DIQNoise : public UVoxelNode_2DIQNoiseBase
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DIM_IMPL(2, _DERIVATIVE, _FRACTAL, IQNoise, FVoxel2DIQNoiseComputeNode)
};

// 3D IQ Noise: uses the derivative to "smooth" the fractals. 
// For more details: 
// http://www.iquilezles.org/www/articles/morenoise/morenoise.htm
// http://www.decarpentier.nl/scape-procedural-basics
UCLASS(DisplayName = "3D IQ Noise", Category = "Noise|IQ Noise")
class VOXELGRAPH_API UVoxelNode_3DIQNoise : public UVoxelNode_3DIQNoiseBase
{
	GENERATED_BODY()
	GENERATED_NOISENODE_BODY_DIM_IMPL(3, _DERIVATIVE, _FRACTAL, IQNoise, FVoxel3DIQNoiseComputeNode)
};