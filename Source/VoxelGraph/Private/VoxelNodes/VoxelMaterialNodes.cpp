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

UVoxelNode_MakeMaterialFromColor::UVoxelNode_MakeMaterialFromColor()
{
	SetInputs({ "Color", EC::Color, "Color" });
	SetOutputs(EC::Material);
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_MakeMaterialFromColor,
	DEFINE_INPUTS(FColor),
	DEFINE_OUTPUTS(FVoxelMaterial),
	_O0 = FVoxelNodeFunctions::MaterialFromColor(_I0);
)

UVoxelNode_GetIndex::UVoxelNode_GetIndex()
{
	SetInputs({ "Material", EC::Material, "Material" });
	SetOutputs(
		{ "Index", EC::Int, "Index between 0 and 255" },
		{ "Data A", EC::Float, "Data sent to material shader" },
		{ "Data B", EC::Float, "Data sent to material shader" },
		{ "Data C", EC::Float, "Data sent to material shader" });
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GetIndex,
	DEFINE_INPUTS(FVoxelMaterial),
	DEFINE_OUTPUTS(int32, v_flt, v_flt, v_flt),
	FVoxelNodeFunctions::SingleIndexFromMaterial(_I0, _O0, _O1, _O2, _O3);
)

UVoxelNode_MakeMaterialFromSingleIndex::UVoxelNode_MakeMaterialFromSingleIndex()
{
	SetInputs(
		{ "Index", EC::Int, "Index between 0 and 255", "", {0, 255} },
		{ "Data A", EC::Float, "Data to send to the material shader", "", {0, 1} },
		{ "Data B", EC::Float, "Data to send to the material shader", "", {0, 1} },
		{ "Data C", EC::Float, "Data to send to the material shader", "", {0, 1} });
	SetOutputs(EC::Material);
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_MakeMaterialFromSingleIndex,
	DEFINE_INPUTS(int32, v_flt, v_flt, v_flt),
	DEFINE_OUTPUTS(FVoxelMaterial),
	_O0 = FVoxelNodeFunctions::MaterialFromSingleIndex(_I0, _I1, _I2, _I3);
)

UVoxelNode_GetDoubleIndex::UVoxelNode_GetDoubleIndex()
{
	SetInputs({ "Material", EC::Material, "Material" });
	SetOutputs(
		{ "Index A", EC::Int, "Index A between 0 and 255" },
		{ "Index B", EC::Int, "Index B between 0 and 255" },
		{ "Blend", EC::Float, "Blend factor, between 0 and 1" },
		{ "Data", EC::Float, "Data sent to material shader" });
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_GetDoubleIndex,
	DEFINE_INPUTS(FVoxelMaterial),
	DEFINE_OUTPUTS(int32, int32, v_flt, v_flt),
	FVoxelNodeFunctions::DoubleIndexFromMaterial(_I0, _O0, _O1, _O2, _O3);
)

UVoxelNode_MakeMaterialFromDoubleIndex::UVoxelNode_MakeMaterialFromDoubleIndex()
{
	SetInputs(
		{ "Index A", EC::Int, "Index A between 0 and 255", "", {0, 255} },
		{ "Index B", EC::Int, "Index B between 0 and 255", "", {0, 255} },
		{ "Blend", EC::Float, "Blend factor, between 0 and 1", "", {0, 1} },
		{ "Data", EC::Float, "Data to send to the material shader", "", {0, 1} });
	SetOutputs(EC::Material);
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_MakeMaterialFromDoubleIndex,
	DEFINE_INPUTS(int32, int32, v_flt, v_flt),
	DEFINE_OUTPUTS(FVoxelMaterial),
	_O0 = FVoxelNodeFunctions::MaterialFromDoubleIndex(_I0, _I1, _I2, _I3);
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

///////////////////////////////////////////////////////////////////////////////

UVoxelNode_CreateDoubleIndexMaterial::UVoxelNode_CreateDoubleIndexMaterial()
{
	SetInputsCount(3, MAX_VOXELNODE_PINS);
	SetInputIncrement(2);
	SetOutputs(EC::Material);
}

EVoxelPinCategory UVoxelNode_CreateDoubleIndexMaterial::GetInputPinCategory(int32 PinIndex) const
{
	if (PinIndex % 2 == 1)
	{
		return EC::Int;
	}
	else
	{
		return EC::Float;
	}
}

FName UVoxelNode_CreateDoubleIndexMaterial::GetInputPinName(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "Data";
	}
	if (PinIndex % 2 == 1)
	{
		return "Index";
	}
	else
	{
		return "Alpha";
	}
}

FString UVoxelNode_CreateDoubleIndexMaterial::GetInputPinDefaultValue(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "";
	}
	if (PinIndex % 2 == 1)
	{
		return "";
	}
	else
	{
		return "1";
	}
}

