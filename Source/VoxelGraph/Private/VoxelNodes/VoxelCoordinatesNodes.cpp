// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelCoordinatesNodes.h"
#include "VoxelNodes/VoxelNodeColors.h"
#include "Runtime/VoxelNodeType.h"
#include "VoxelContext.h"
#include "VoxelAxisDependencies.h"
#include "VoxelNodeFunctions.h"

UVoxelCoordinateNode::UVoxelCoordinateNode()
{
	SetColor(FVoxelNodeColors::FloatNode);
}

//////////////////////////////////////////////////////////////////////////////////////

// Note: Both local and global coordinates are depending on their corresponding axis.
// If the transform has a rotation, the graph generator helper won't use the axis dependencies

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_XF::UVoxelNode_XF()
{
	SetOutputs(EC::Float);
}
uint8 UVoxelNode_XF::GetNodeDependencies() const
{
	return EVoxelAxisDependenciesFlags::X;
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_XF,
	NO_INPUTS,
	DEFINE_OUTPUTS(v_flt),
	_O0 = _C0.GetLocalX();
)

UVoxelNode_YF::UVoxelNode_YF()
{
	SetOutputs(EC::Float);
}
uint8 UVoxelNode_YF::GetNodeDependencies() const
{
	return EVoxelAxisDependenciesFlags::Y;
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_YF,
	NO_INPUTS,
	DEFINE_OUTPUTS(v_flt),
	_O0 = _C0.GetLocalY();
)

UVoxelNode_ZF::UVoxelNode_ZF()
{
	SetOutputs(EC::Float);
}
uint8 UVoxelNode_ZF::GetNodeDependencies() const
{
	return EVoxelAxisDependenciesFlags::Z;
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_ZF,
	NO_INPUTS,
	DEFINE_OUTPUTS(v_flt),
	_O0 = _C0.GetLocalZ();
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_GlobalX::UVoxelNode_GlobalX()
{
	SetOutputs(EC::Float);
}
uint8 UVoxelNode_GlobalX::GetNodeDependencies() const
{
	return EVoxelAxisDependenciesFlags::X;
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GlobalX,
	NO_INPUTS,
	DEFINE_OUTPUTS(v_flt),
	_O0 = _C0.GetWorldX();
)

UVoxelNode_GlobalY::UVoxelNode_GlobalY()
{
	SetOutputs(EC::Float);
}
uint8 UVoxelNode_GlobalY::GetNodeDependencies() const
{
	return EVoxelAxisDependenciesFlags::Y;
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GlobalY,
	NO_INPUTS,
	DEFINE_OUTPUTS(v_flt),
	_O0 = _C0.GetWorldY();
)

UVoxelNode_GlobalZ::UVoxelNode_GlobalZ()
{
	SetOutputs(EC::Float);
}
uint8 UVoxelNode_GlobalZ::GetNodeDependencies() const
{
	return EVoxelAxisDependenciesFlags::Z;
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GlobalZ,
	NO_INPUTS,
	DEFINE_OUTPUTS(v_flt),
	_O0 = _C0.GetWorldZ();
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_LocalToGlobal::UVoxelNode_LocalToGlobal()
{
	SetInputs(
		{ "X", EC::Float, "X in local space" },
		{ "Y", EC::Float, "Y in local space" },
		{ "Z", EC::Float, "Z in local space" });
	SetOutputs(
		{ "X", EC::Float, "X in global space" },
		{ "Y", EC::Float, "Y in global space" },
		{ "Z", EC::Float, "Z in global space" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_LocalToGlobal,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::LocalToGlobal(_I0, _I1, _I2, _O0, _O1, _O2, _C0);
)

UVoxelNode_GlobalToLocal::UVoxelNode_GlobalToLocal()
{
	SetInputs(
		{ "X", EC::Float, "X in global space" },
		{ "Y", EC::Float, "Y in global space" },
		{ "Z", EC::Float, "Z in global space" });
	SetOutputs(
		{ "X", EC::Float, "X in local space" },
		{ "Y", EC::Float, "Y in local space" },
		{ "Z", EC::Float, "Z in local space" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GlobalToLocal,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::GlobalToLocal(_I0, _I1, _I2, _O0, _O1, _O2, _C0);
)

//////////////////////////////////////////////////////////////////////////////////////

UVoxelNode_TransformVector::UVoxelNode_TransformVector()
{
	SetInputs(
		{ "X", EC::Float, "X in local space" },
		{ "Y", EC::Float, "Y in local space" },
		{ "Z", EC::Float, "Z in local space" });
	SetOutputs(
		{ "X", EC::Float, "X in global space" },
		{ "Y", EC::Float, "Y in global space" },
		{ "Z", EC::Float, "Z in global space" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_TransformVector,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::TransformVector(_I0, _I1, _I2, _O0, _O1, _O2, _C0);
)

UVoxelNode_InverseTransformVector::UVoxelNode_InverseTransformVector()
{
	SetInputs(
		{ "X", EC::Float, "X in global space" },
		{ "Y", EC::Float, "Y in global space" },
		{ "Z", EC::Float, "Z in global space" });
	SetOutputs(
		{ "X", EC::Float, "X in local space" },
		{ "Y", EC::Float, "Y in local space" },
		{ "Z", EC::Float, "Z in local space" });
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_InverseTransformVector,
	DEFINE_INPUTS(v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::InverseTransformVector(_I0, _I1, _I2, _O0, _O1, _O2, _C0);
)