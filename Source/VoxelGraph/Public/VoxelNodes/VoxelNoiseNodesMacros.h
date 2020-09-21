// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "FastNoise/VoxelFastNoise.inl"

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
	GENERATED_NOISENODE_BODY_DIM_IMPL(2, _DERIV,, FunctionName, FVoxelNoiseComputeNode)
#define GENERATED_NOISENODE_BODY_DERIVATIVE_DIM3(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(3, _DERIV,, FunctionName, FVoxelNoiseComputeNode)

#define GENERATED_NOISENODE_BODY_FRACTAL_DERIVATIVE_DIM2(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(2, _DERIV, _FRACTAL, FunctionName, FVoxelNoiseFractalComputeNode)
#define GENERATED_NOISENODE_BODY_FRACTAL_DERIVATIVE_DIM3(FunctionName) \
	GENERATED_NOISENODE_BODY_DIM_IMPL(3, _DERIV, _FRACTAL, FunctionName, FVoxelNoiseFractalComputeNode)