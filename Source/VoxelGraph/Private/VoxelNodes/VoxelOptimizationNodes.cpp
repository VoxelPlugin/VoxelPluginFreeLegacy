// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelOptimizationNodes.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelContext.h"
#include "VoxelNodeFunctions.h"
#include "VoxelMessages.h"
#include "VoxelGraphGenerator.h"

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