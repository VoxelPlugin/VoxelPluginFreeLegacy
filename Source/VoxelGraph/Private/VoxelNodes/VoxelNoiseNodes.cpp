// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelNoiseNodes.h"
#include "VoxelGraphGenerator.h"
#include "VoxelMathUtilities.h"

#define NOISE_SAMPLE_RANGE 10

FName UVoxelNode_NoiseNode::GetInputPinName(int32 PinIndex) const
{
	if (GetDimension() == 2)
	{
		if (PinIndex == 0)
		{
			return "X";
		}
		else if (PinIndex == 1)
		{
			return "Y";
		}
		else if (PinIndex == 2)
		{
			return "Frequency";
		}
		else
		{
			return "Seed";
		}
	}
	else
	{
		if (PinIndex == 0)
		{
			return "X";
		}
		else if (PinIndex == 1)
		{
			return "Y";
		}
		else if (PinIndex == 2)
		{
			return "Z";
		}
		else if (PinIndex == 3)
		{
			return "Frequency";
		}
		else
		{
			return "Seed";
		}
	}
}

FName UVoxelNode_NoiseNode::GetOutputPinName(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return IsDerivative() ? "Value" : "";
	}
	else if (PinIndex == 1)
	{
		return "DX";
	}
	else if (PinIndex == 2)
	{
		return "DY";
	}
	else
	{
		return "DZ";
	}
}

FString UVoxelNode_NoiseNode::GetInputPinToolTip(int32 PinIndex) const
{
	if (GetDimension() == 2)
	{
		if (PinIndex == 0)
		{
			return "X";
		}
		else if (PinIndex == 1)
		{
			return "Y";
		}
		else if (PinIndex == 2)
		{
			return "The frequency of the noise";
		}
		else
		{
			return "The seed to use";
		}
	}
	else
	{
		if (PinIndex == 0)
		{
			return "X";
		}
		else if (PinIndex == 1)
		{
			return "Y";
		}
		else if (PinIndex == 2)
		{
			return "Z";
		}
		else if (PinIndex == 3)
		{
			return "The frequency of the noise";
		}
		else
		{
			return "The seed to use";
		}
	}
}

FString UVoxelNode_NoiseNode::GetOutputPinToolTip(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "The noise value";
	}
	else if (PinIndex == 1)
	{
		return "The derivative along the X axis. Can be used to compute the slope of the noise using GetSlopeFromDerivatives.";
	}
	else if (PinIndex == 2)
	{
		return "The derivative along the Y axis. Can be used to compute the slope of the noise using GetSlopeFromDerivatives.";
	}
	else
	{
		return "The derivative along the Z axis. Can be used to compute the slope of the noise using GetSlopeFromDerivatives.";
	}
}

FString UVoxelNode_NoiseNode::GetInputPinDefaultValue(int32 PinIndex) const
{
	if (PinIndex == GetDimension())
	{
		return FString::SanitizeFloat(Frequency);
	}
	else
	{
		return "";
	}
}

///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelNode_NoiseNode::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty &&
		PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
	}
}

bool UVoxelNode_NoiseNode::CanEditChange(const UProperty* InProperty) const
{
	return
		Super::CanEditChange(InProperty) && 
		(NeedRangeAnalysis() || InProperty->GetFName() != GET_MEMBER_NAME_STATIC(UVoxelNode_NoiseNode, NumberOfSamples));
}
#endif

void UVoxelNode_NoiseNode::PostLoad()
{
	Super::PostLoad();
	if (OutputRanges.Num() == 0)
	{
	}
}

void UVoxelNode_NoiseNode::PostInitProperties()
{
	Super::PostInitProperties();
	if (OutputRanges.Num() == 0 && !HasAnyFlags(RF_ClassDefaultObject | RF_NeedLoad))
	{
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FLinearColor UVoxelNode_NoiseNodeFractal::GetNodeBodyColor() const
{
	return FMath::Lerp<FLinearColor>(FColorList::White, FColorList::Orange, FMath::Clamp<float>((FractalOctaves - 1.f) / 10, 0, 1));
}

FLinearColor UVoxelNode_NoiseNodeFractal::GetColor() const
{
	return FMath::Lerp<FLinearColor>(FColorList::Black, FColorList::Orange, FMath::Clamp<float>((FractalOctaves - 1.f) / 10, 0, 1));
}

///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelNode_NoiseNodeFractal::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		LODToOctavesMap.Add("0", FractalOctaves);
		int32 MaxInt = 0;
		for (auto& It : LODToOctavesMap)
		{
			if (!It.Key.IsEmpty())
			{
				const int32 Int = FVoxelUtilities::ClampDepth<RENDER_CHUNK_SIZE>(TCString<TCHAR>::Atoi(*It.Key));
				MaxInt = FMath::Max(MaxInt, Int);
				It.Key = FString::FromInt(Int);
			}
		}
		for (auto& It : LODToOctavesMap)
		{
			if (It.Key.IsEmpty())
			{
				if (uint8 * LOD = LODToOctavesMap.Find(FString::FromInt(MaxInt)))
				{
					It.Value = *LOD - 1;
				}
				It.Key = FString::FromInt(++MaxInt);
			}
		}
		LODToOctavesMap.KeySort([](const FString& A, const FString& B) { return TCString<TCHAR>::Atoi(*A) < TCString<TCHAR>::Atoi(*B); });
	}
}
#endif

void UVoxelNode_NoiseNodeFractal::PostLoad()
{
	LODToOctavesMap.Add("0", FractalOctaves);
	Super::PostLoad(); // Make sure to call ComputeRange after
}

void UVoxelNode_NoiseNodeFractal::PostInitProperties()
{
	LODToOctavesMap.Add("0", FractalOctaves);
	Super::PostInitProperties(); // Make sure to call ComputeRange after
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_IQNoiseBase::UVoxelNode_IQNoiseBase()
{
	bComputeDerivative = true;
	FractalType = EFractalType::FBM;
	FractalOctaves = 15;
	Frequency = 0.001;
	NumberOfSamples = 1000000;
}

#if WITH_EDITOR
bool UVoxelNode_IQNoiseBase::CanEditChange(const UProperty* InProperty) const
{
	return
		Super::CanEditChange(InProperty) &&
		InProperty->GetFName() != GET_MEMBER_NAME_STATIC(UVoxelNode_IQNoiseBase, bComputeDerivative) &&
		InProperty->GetFName() != GET_MEMBER_NAME_STATIC(UVoxelNode_IQNoiseBase, FractalType);
}
#endif


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelNode_NoiseNodeWithDerivative::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName()  == GET_MEMBER_NAME_STATIC(UVoxelNode_NoiseNodeWithDerivative, bComputeDerivative) &&
		PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive &&
		Graph &&
		GraphNode)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}

	// Reconstruct first to have the right output count
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UVoxelNode_NoiseNodeWithDerivativeFractal::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.Property->GetFName()  == GET_MEMBER_NAME_STATIC(UVoxelNode_NoiseNodeWithDerivative, bComputeDerivative) &&
		PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive &&
		Graph &&
		GraphNode)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}

	// Reconstruct first to have the right output count
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif