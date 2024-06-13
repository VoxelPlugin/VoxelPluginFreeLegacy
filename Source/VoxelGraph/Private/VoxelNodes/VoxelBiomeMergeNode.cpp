// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelNodes/VoxelBiomeMergeNode.h"
#include "VoxelNodes/VoxelNodeHelperMacros.h"
#include "VoxelGraphGenerator.h"

int32 UVoxelNode_BiomeMerge::GetMinInputPins() const
{
	return 1 + Biomes.Num() * 2;
}

int32 UVoxelNode_BiomeMerge::GetMaxInputPins() const
{
	return GetMinInputPins();
}

int32 UVoxelNode_BiomeMerge::GetOutputPinsCount() const
{
	return 2;
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
	return EC::Float;
}


#if WITH_EDITOR
void UVoxelNode_BiomeMerge::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
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
	}
	
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif