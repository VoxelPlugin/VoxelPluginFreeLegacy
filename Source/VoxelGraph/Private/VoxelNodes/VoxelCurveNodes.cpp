// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelCurveNodes.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelNodeFunctions.h"

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

void UVoxelNode_Curve::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	if (!Curve)
	{
		ErrorReporter.AddMessageToNode(this, "invalid curve", EVoxelGraphNodeMessageType::FatalError);
	}
}

#if WITH_EDITOR
bool UVoxelNode_Curve::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	return TryImportObject(Property, Object, Curve);
}
#endif

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

void UVoxelNode_CurveColor::LogErrors(FVoxelGraphErrorReporter& ErrorReporter)
{
	Super::LogErrors(ErrorReporter);
	if (!Curve)
	{
		ErrorReporter.AddMessageToNode(this, "invalid color curve", EVoxelGraphNodeMessageType::FatalError);
	}
}

#if WITH_EDITOR
bool UVoxelNode_CurveColor::TryImportFromProperty(UProperty* Property, UObject* Object)
{
	return TryImportObject(Property, Object, Curve);
}
#endif