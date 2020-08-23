// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNoiseNodesBase.h"
#include "VoxelGradientPerturbNodes.generated.h"

template<typename Parent>
class TVoxelGradientPerturbHelper : public Parent
{
public:
	using Parent::Parent;
	
	//~ Begin UVoxelNode Interface
	virtual FName GetInputPinName(int32 PinIndex) const override { return UVoxelNodeHelper::GetInputPinName(PinIndex); }
	virtual FName GetOutputPinName(int32 PinIndex) const override { return UVoxelNodeHelper::GetOutputPinName(PinIndex); }
	virtual FString GetInputPinToolTip(int32 PinIndex) const override { return UVoxelNodeHelper::GetInputPinToolTip(PinIndex); }
	virtual FString GetOutputPinToolTip(int32 PinIndex) const override { return UVoxelNodeHelper::GetOutputPinToolTip(PinIndex); }
	virtual int32 GetMinInputPins() const override { return UVoxelNodeHelper::GetMinInputPins(); }
	virtual int32 GetMaxInputPins() const override { return UVoxelNodeHelper::GetMaxInputPins(); }
	virtual int32 GetOutputPinsCount() const override { return UVoxelNodeHelper::GetOutputPinsCount(); }
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override { return UVoxelNodeHelper::GetInputPinCategory(PinIndex); }
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override { return UVoxelNodeHelper::GetOutputPinCategory(PinIndex); }
	virtual FString GetInputPinDefaultValue(int32 PinIndex) const override { return UVoxelNodeHelper::GetInputPinDefaultValue(PinIndex); }
	//~ End UVoxelNode Interface
	
	//~ Begin UVoxelNode_NoiseNode Interface
	virtual bool NeedRangeAnalysis() const override { return false; }
	//~ End UVoxelNode_NoiseNode Interface
};

#define UVoxelNode_NoiseNode TVoxelGradientPerturbHelper<UVoxelNode_NoiseNode>
#define UVoxelNode_NoiseNodeFractal TVoxelGradientPerturbHelper<UVoxelNode_NoiseNodeFractal>

UCLASS(Abstract, Category = "Noise|Gradient Perturb")
class VOXELGRAPH_API UVoxelNode_GradientPerturb : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
};

UCLASS(Abstract, Category = "Noise|Gradient Perturb")
class VOXELGRAPH_API UVoxelNode_GradientPerturbFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
};

#undef UVoxelNode_NoiseNode
#undef UVoxelNode_NoiseNodeFractal

// 2D Gradient Perturb
UCLASS(DisplayName = "2D Gradient Perturb")
class VOXELGRAPH_API UVoxelNode_2DGradientPerturb : public UVoxelNode_GradientPerturb
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_2DGradientPerturb();
	virtual uint32 GetDimension() const override { return 2; }
};

// 2D Gradient Perturb Fractal
UCLASS(DisplayName = "2D Gradient Perturb Fractal")
class VOXELGRAPH_API UVoxelNode_2DGradientPerturbFractal : public UVoxelNode_GradientPerturbFractal
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_2DGradientPerturbFractal();
	virtual uint32 GetDimension() const override { return 2; }
};

// 3D Gradient Perturb
UCLASS(DisplayName = "3D Gradient Perturb")
class VOXELGRAPH_API UVoxelNode_3DGradientPerturb : public UVoxelNode_GradientPerturb
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_3DGradientPerturb();
	virtual uint32 GetDimension() const override { return 3; }
};

// 3D Gradient Perturb Fractal
UCLASS(DisplayName = "3D Gradient Perturb Fractal")
class VOXELGRAPH_API UVoxelNode_3DGradientPerturbFractal : public UVoxelNode_GradientPerturbFractal
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_3DGradientPerturbFractal();
	virtual uint32 GetDimension() const override { return 3; }
};