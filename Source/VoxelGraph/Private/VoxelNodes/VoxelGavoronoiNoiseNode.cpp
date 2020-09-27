// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelGavoronoiNoiseNode.h"

inline void AddGavoronoiPins(FVoxelPinsHelper& Pins)
{
	Pins.InputPins.Add(FVoxelHelperPin("Direction X", EVoxelPinCategory::Float, "Direction of the noise", "0.5"));
	Pins.InputPins.Add(FVoxelHelperPin("Direction Y", EVoxelPinCategory::Float, "Direction of the noise", "0.5"));
	Pins.InputPins.Add(FVoxelHelperPin("Direction Variation", EVoxelPinCategory::Float, "Strength of the noise added to the direction, between 0 and 1", "0.4"));
}

UVoxelNode_2DGavoronoiNoise::UVoxelNode_2DGavoronoiNoise()
{
	AddGavoronoiPins(CustomNoisePins);
}

UVoxelNode_2DGavoronoiNoiseFractal::UVoxelNode_2DGavoronoiNoiseFractal()
{
	AddGavoronoiPins(CustomNoisePins);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_2DErosion::UVoxelNode_2DErosion()
{
	bComputeDerivative = true;

	const FString ToolTip = "The derivative of the noise to erode. You can get them by ticking Compute Derivatives in the node details";
	
	CustomNoisePins.InputPins.Add(FVoxelHelperPin("Noise DX", EVoxelPinCategory::Float, ToolTip));
	CustomNoisePins.InputPins.Add(FVoxelHelperPin("Noise DY", EVoxelPinCategory::Float, ToolTip));
}


#if WITH_EDITOR
bool UVoxelNode_2DErosion::CanEditChange(const FProperty* InProperty) const
{
	return
		Super::CanEditChange(InProperty) &&
		InProperty->GetFName() != GET_MEMBER_NAME_STATIC(UVoxelNode_2DErosion, bComputeDerivative) &&
		InProperty->GetFName() != GET_MEMBER_NAME_STATIC(UVoxelNode_2DErosion, FractalType);
}
#endif

