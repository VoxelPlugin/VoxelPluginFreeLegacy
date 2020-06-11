// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelMaterialNodes.h"
#include "Runtime/VoxelNodeType.h"
#include "VoxelContext.h"
#include "VoxelNodeFunctions.h"

UVoxelNode_GetColor::UVoxelNode_GetColor()
{
	SetInputs({ "Material", EC::Material, "Material" });
	SetOutputs({ "Color", EC::Color, "Material color" });
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GetColor,
	DEFINE_INPUTS(FVoxelMaterial),
	DEFINE_OUTPUTS(FColor),
	_O0 = FVoxelNodeFunctions::ColorFromMaterial(_I0);
)

UVoxelNode_GetIndex::UVoxelNode_GetIndex()
{
	SetInputs({ "Material", EC::Material, "Material" });
	SetOutputs(
		{ "Index", EC::Int, "Index between 0 and 255" });
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GetIndex,
	DEFINE_INPUTS(FVoxelMaterial),
	DEFINE_OUTPUTS(int32, v_flt, v_flt, v_flt),
	_O0 = FVoxelNodeFunctions::SingleIndexFromMaterial(_I0);
)

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_GetUVChannel::UVoxelNode_GetUVChannel()
{
	SetInputs(
		{ "Material", EC::Material, "The material" },
		{ "Channel", EC::Int, "The UV channel", "", {0, 255} });
	SetOutputs(
		{ "U", EC::Float, "U coordinate, between 0 and 1" },
		{ "V", EC::Float, "V coordinate, between 0 and 1" });
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GetUVChannel,
	DEFINE_INPUTS(FVoxelMaterial, int32),
	DEFINE_OUTPUTS(v_flt, v_flt),
	FVoxelNodeFunctions::GetUVChannelFromMaterial(_I0, _I1, _O0, _O1);
)