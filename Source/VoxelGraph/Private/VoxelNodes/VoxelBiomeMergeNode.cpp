// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelBiomeMergeNode.h"
#include "VoxelNodes/VoxelNodeHelperMacros.h"
#include "VoxelGraphGenerator.h"

int32 UVoxelNode_BiomeMerge::GetMinInputPins() const
{
	return 1 + Biomes.Num() * 2 + AdditionalData.Num();
}

int32 UVoxelNode_BiomeMerge::GetMaxInputPins() const
{
	return GetMinInputPins();
}

int32 UVoxelNode_BiomeMerge::GetOutputPinsCount() const
{
	return 1 + 1 + AdditionalData.Num();
}

FName UVoxelNode_BiomeMerge::GetInputPinName(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "";
	}
	PinIndex--;
	if (PinIndex < 2 * Biomes.Num())
	{
		if (PinIndex % 2 == 0)
		{
			return *(Biomes[PinIndex / 2] + " Value");
		}
		else
		{
			return *(Biomes[PinIndex / 2] + " Alpha");
		}
	}
	PinIndex -= 2 * Biomes.Num();
	if (AdditionalData.IsValidIndex(PinIndex))
	{
		return *AdditionalData[PinIndex].Name;
	}
	return "Error";
}

FName UVoxelNode_BiomeMerge::GetOutputPinName(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return "";
	}
	if (PinIndex == 1)
	{
		return "Result";
	}
	PinIndex -= 2;
	if (AdditionalData.IsValidIndex(PinIndex))
	{
		return *AdditionalData[PinIndex].Name;
	}
	return "Error";
}

EVoxelPinCategory UVoxelNode_BiomeMerge::GetInputPinCategory(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return EC::Exec;
	}
	PinIndex--;
	if (PinIndex < 2 * Biomes.Num())
	{
		return EC::Float;
	}
	PinIndex -= 2 * Biomes.Num();
	if (AdditionalData.IsValidIndex(PinIndex))
	{
		return FVoxelPinCategory::DataPinToPin(AdditionalData[PinIndex].Type);
	}
	return EC::Float;
}

EVoxelPinCategory UVoxelNode_BiomeMerge::GetOutputPinCategory(int32 PinIndex) const
{
	if (PinIndex == 0)
	{
		return EC::Exec;
	}
	if (PinIndex == 1)
	{
		return EC::Float;
	}
	PinIndex -= 2;
	if (AdditionalData.IsValidIndex(PinIndex))
	{
		return FVoxelPinCategory::DataPinToPin(AdditionalData[PinIndex].Type);
	}
	return EC::Float;
}


#if WITH_EDITOR
void UVoxelNode_BiomeMerge::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (Graph && GraphNode && PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		if (Biomes.Num() >= 256)
		{
			Biomes.SetNum(256);
		}
		for (int32 Index = 0; Index < Biomes.Num(); Index++)
		{
			auto& Biome = Biomes[Index];
			if (Biome.IsEmpty())
			{
				Biome = FString::Printf(TEXT("Biome %d"), Index);
			}
		}

		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif