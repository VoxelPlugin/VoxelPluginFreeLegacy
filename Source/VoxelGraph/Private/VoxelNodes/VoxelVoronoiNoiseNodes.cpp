// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelVoronoiNoiseNodes.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelGraphGenerator.h"
#include "FastNoise.h"

int32 UVoxelNode_VoronoiNoiseBase::GetMinInputPins() const
{
	return GetPins().InputPins.Num();
}

int32 UVoxelNode_VoronoiNoiseBase::GetMaxInputPins() const
{
	return GetMinInputPins();
}

int32 UVoxelNode_VoronoiNoiseBase::GetOutputPinsCount() const
{
	return GetPins().OutputPins.Num();
}

EVoxelPinCategory UVoxelNode_VoronoiNoiseBase::GetInputPinCategory(int32 PinIndex) const
{
	return GetPins().GetInputPin(PinIndex).Category;
}

EVoxelPinCategory UVoxelNode_VoronoiNoiseBase::GetOutputPinCategory(int32 PinIndex) const
{
	return GetPins().GetOutputPin(PinIndex).Category;
}

FName UVoxelNode_VoronoiNoiseBase::GetInputPinName(int32 PinIndex) const
{
	return GetPins().GetInputPin(PinIndex).Name;
}

FName UVoxelNode_VoronoiNoiseBase::GetOutputPinName(int32 PinIndex) const
{
	return GetPins().GetOutputPin(PinIndex).Name;
}

FString UVoxelNode_VoronoiNoiseBase::GetInputPinToolTip(int32 PinIndex) const
{
	return GetPins().GetInputPin(PinIndex).ToolTip;
}

FString UVoxelNode_VoronoiNoiseBase::GetOutputPinToolTip(int32 PinIndex) const
{
	return GetPins().GetOutputPin(PinIndex).ToolTip;
}

FVoxelPinDefaultValueBounds UVoxelNode_VoronoiNoiseBase::GetInputPinDefaultValueBounds(int32 PinIndex) const
{
	return GetPins().GetInputPin(PinIndex).DefaultValueBounds;
}

FString UVoxelNode_VoronoiNoiseBase::GetInputPinDefaultValue(int32 PinIndex) const
{
	return GetPins().GetInputPin(PinIndex).DefaultValue;
}

#if WITH_EDITOR
void UVoxelNode_VoronoiNoiseBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property &&
		PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive &&
		PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_STATIC(UVoxelNode_VoronoiNoiseBase, bComputeNeighbors) &&
		Graph &&
		GraphNode)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif

const FVoxelPinsHelper& UVoxelNode_VoronoiNoiseBase::GetPins() const
{
	static const TArray<FVoxelHelperPin> InputPinsDim2 =
	{
		{"X", EC::Float, "X"},
		{"Y", EC::Float, "Y"},
		{"Jitter", EC::Float, "Jitter of the noise. Increase this to make the noise less blocky", "0.45"},
		{"Seed", EC::Seed, "Seed"}
	};
	static const TArray<FVoxelHelperPin> InputPinsDim3 =
	{
		{"X", EC::Float, "X"},
		{"Y", EC::Float, "Y"},
		{"Z", EC::Float, "Z"},
		{"Jitter", EC::Float, "Jitter of the noise. Increase this to make the noise less blocky", "0.45"},
		{"Seed", EC::Seed, "Seed"}
	};
	static const TArray<FVoxelHelperPin> OutputPinsDim2 =
	{
		{"X", EC::Float, "X"},
		{"Y", EC::Float, "Y"}
	};
	static const TArray<FVoxelHelperPin> OutputPinsDim3 =
	{
		{"X", EC::Float, "X"},
		{"Y", EC::Float, "Y"},
		{"Z", EC::Float, "Z"}
	};
	static const TArray<FVoxelHelperPin> OutputPinsDim2Neighbors =
	{
		{"X 0", EC::Float, "X coordinate of the closest cell"},
		{"Y 0", EC::Float, "Y coordinate of the closest cell"},
		{"X 1", EC::Float, "X coordinate of the second closest cell"},
		{"Y 1", EC::Float, "Y coordinate of the second closest cell"},
		{"Distance 1", EC::Float, "Distance to the border of second closest cell"},
		{"X 2", EC::Float, "X coordinate of the third closest cell"},
		{"Y 2", EC::Float, "Y coordinate of the third closest cell"},
		{"Distance 2", EC::Float, "Distance to the border of the third closest cell"},
		{"X 3", EC::Float, "X coordinate of the fourth closest cell"},
		{"Y 3", EC::Float, "Y coordinate of the fourth closest cell"},
		{"Distance 3", EC::Float, "Distance to the border of the fourth closest cell"}
	};
	static const TArray<FVoxelHelperPin> OutputPinsDim3Neighbors =
	{
	};

	static const FVoxelPinsHelper PinsDim2 = { InputPinsDim2, OutputPinsDim2 };
	static const FVoxelPinsHelper PinsDim3 = { InputPinsDim3, OutputPinsDim3 };
	static const FVoxelPinsHelper PinsDim2Neighbors = { InputPinsDim2, OutputPinsDim2Neighbors };
	static const FVoxelPinsHelper PinsDim3Neighbors = { InputPinsDim3, OutputPinsDim3Neighbors };

	if (bComputeNeighbors)
	{
		return Dimension == 2 ? PinsDim2Neighbors : PinsDim3Neighbors;
	}
	else
	{
		return Dimension == 2 ? PinsDim2 : PinsDim3;
	}
}


UVoxelNode_2DVoronoiNoise::UVoxelNode_2DVoronoiNoise()
{
	Dimension = 2;
}

