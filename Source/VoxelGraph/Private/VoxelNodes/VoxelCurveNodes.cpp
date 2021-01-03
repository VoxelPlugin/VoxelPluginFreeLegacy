// Copyright 2021 Phyronnaz

#include "VoxelNodes/VoxelCurveNodes.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "NodeFunctions/VoxelNodeFunctions.h"

#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"

UVoxelNode_Curve::UVoxelNode_Curve()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}


FText UVoxelNode_Curve::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Float Curve: {0}"), Super::GetTitle());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_CurveColor::UVoxelNode_CurveColor()
{
	SetInputs(EC::Float);
	SetOutputs(
		{ "R", EC::Float, "Red between 0 and 1" },
		{ "G", EC::Float, "Green between 0 and 1" },
		{ "B", EC::Float, "Blue between 0 and 1" },
		{ "A", EC::Float, "Alpha between 0 and 1" });
}


FText UVoxelNode_CurveColor::GetTitle() const
{
	return FText::Format(VOXEL_LOCTEXT("Color Curve: {0}"), Super::GetTitle());
}