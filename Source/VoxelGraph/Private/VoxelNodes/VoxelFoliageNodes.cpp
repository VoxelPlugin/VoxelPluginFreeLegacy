// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelFoliageNodes.h"
#include "NodeFunctions/VoxelFoliageNodeFunctions.h"

UVoxelNode_SampleFoliageMaterialIndex::UVoxelNode_SampleFoliageMaterialIndex()
{
	SetInputs({ "Index", EVoxelPinCategory::Int, "Index to sample" });
	SetOutputs({ "Value", EVoxelPinCategory::Float, "Between 0 and 1" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_SampleFoliageMaterialIndex,
	DEFINE_INPUTS(int32),
	DEFINE_OUTPUTS(v_flt),
	Output0 = FVoxelFoliageNodeFunctions::SampleFoliageMaterialIndex(Input0, Context);
)