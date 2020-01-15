// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelPlaceableItemsNodes.h"
#include "Runtime/VoxelNodeType.h"
#include "VoxelContext.h"
#include "VoxelNodeFunctions.h"

UVoxelNode_PerlinWormDistance::UVoxelNode_PerlinWormDistance()
{
	SetInputs(
		{ "X", EC::Float, "X" },
		{ "Y", EC::Float, "Y" },
		{ "Z", EC::Float, "Z" });
	SetOutputs(EC::Float);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_PerlinWormDistance,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt),
	_O0 = FVoxelNodeFunctions::GetPerlinWormsDistance(_C0.Items.ItemHolder, _I0, _I1, _I2);
)