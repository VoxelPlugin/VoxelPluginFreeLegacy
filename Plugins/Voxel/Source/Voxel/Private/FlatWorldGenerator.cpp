// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"

UFlatWorldGenerator::UFlatWorldGenerator() : Height(0), DefaultColor(FColor::White), ValueMultiplier(1)
{

}

float UFlatWorldGenerator::GetDefaultValue_Implementation(FIntVector Position)
{
	return (Position.Z > Height) ? ValueMultiplier : -ValueMultiplier;
}

FColor UFlatWorldGenerator::GetDefaultColor_Implementation(FIntVector Position)
{
	for (auto Layer : Layers)
	{
		if (Layer.Start <= Position.Z && Position.Z < Layer.Start + Layer.Height)
		{
			return Layer.Color.ToFColor(true);
		}
	}
	return DefaultColor.ToFColor(true);
}

void UFlatWorldGenerator::SetVoxelWorld_Implementation(AVoxelWorld* VoxelWorld)
{

}
