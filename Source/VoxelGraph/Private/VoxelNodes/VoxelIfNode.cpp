// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelNodes/VoxelIfNode.h"
#include "VoxelNodes/VoxelNodeColors.h"

UVoxelNode_If::UVoxelNode_If()
{
	SetInputs(EC::Exec, EC::Boolean);
	SetOutputs(
		{ "True", EC::Exec, "Branch used if condition is true" },
		{ "False", EC::Exec, "Branch used if condition is false" });
	SetColor(FVoxelNodeColors::ExecNode);
}

