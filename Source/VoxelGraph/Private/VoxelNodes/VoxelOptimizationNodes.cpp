// Copyright 2021 Phyronnaz

#include "VoxelNodes/VoxelOptimizationNodes.h"
#include "VoxelContext.h"
#include "VoxelMessages.h"
#include "VoxelGraphConstants.h"
#include "VoxelGraphGenerator.h"
#include "NodeFunctions/VoxelNodeFunctions.h"

#include "Async/Async.h"

UVoxelNode_StaticClampFloat::UVoxelNode_StaticClampFloat()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}

FText UVoxelNode_StaticClampFloat::GetTitle() const
{
	return FText::FromString("Static Clamp: " + FString::SanitizeFloat(Min) + " <= X <= " + FString::SanitizeFloat(Max));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_RangeAnalysisDebuggerFloat::UVoxelNode_RangeAnalysisDebuggerFloat()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}

void UVoxelNode_RangeAnalysisDebuggerFloat::UpdateFromBin()
{
	if (Bins.IsValid())
	{
		Min = Bins->bMinMaxInit ? Bins->MinValue : 0;
		Max = Bins->bMinMaxInit ? Bins->MaxValue : 0;
	}
}

void UVoxelNode_RangeAnalysisDebuggerFloat::UpdateGraph()
{
	Curve.GetRichCurve()->Reset();
	Bins->AddToCurve(*Curve.GetRichCurve());
}

void UVoxelNode_RangeAnalysisDebuggerFloat::Reset()
{
	Bins = MakeUnique<FVoxelBins>(GraphMin, GraphMax, GraphStep);
	UpdateFromBin();
}

#if WITH_EDITOR
void UVoxelNode_RangeAnalysisDebuggerFloat::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		Reset();
		UpdateFromBin();
	}
}

void UVoxelNode_RangeAnalysisDebuggerFloat::PostLoad()
{
	Super::PostLoad();
	Reset();
	UpdateFromBin();
}
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_Sleep::UVoxelNode_Sleep()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_RangeUnion::UVoxelNode_RangeUnion()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
	SetInputsCount(2, MAX_VOXELNODE_PINS);
}

GENERATED_VOXELNODE_IMPL_PREFIXOPLOOP(UVoxelNode_RangeUnion, FVoxelNodeFunctions::Union, v_flt)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_IsSingleBool::UVoxelNode_IsSingleBool()
{
	SetInputs(EC::Boolean);
	SetOutputs(EC::Boolean);
}

GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_IsSingleBool,
	DEFINE_INPUTS(bool),
	DEFINE_OUTPUTS(bool),
	_O0 = FVoxelNodeFunctions::IsSingleBool(_I0);
)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_GetRangeAnalysis::UVoxelNode_GetRangeAnalysis()
{
	SetInputs(EC::Float);
	AddOutput("Min", "The min value of the input value for the current voxel");
	AddOutput("Max", "The max value of the input value for the current voxel");
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SmartMin::UVoxelNode_SmartMin()
{
	SetInputs(EC::Float);
	SetOutputs(EC::Float);
	SetInputsCount(3, MAX_VOXELNODE_PINS);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_SmartMax::UVoxelNode_SmartMax()
{
	SetInputs(EC::Exec, EC::Float);
	SetOutputs(EC::Exec, EC::Float);
	SetInputsCount(3, MAX_VOXELNODE_PINS);
}

