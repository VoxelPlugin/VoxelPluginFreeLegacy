// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "FlatWorldGenerator.h"

int UFlatWorldGenerator::GetDefaultValue(FIntVector Position)
{
	return (Position.Z == Height) ? 0 : ((Position.Z > Height) ? 100 : -100);
}

FColor UFlatWorldGenerator::GetDefaultColor(FIntVector Position)
{
	for (auto Layer : Layers)
	{
		if (Layer.Start <= Position.Z && Position.Z < Layer.Start + Layer.Height)
		{
			return Layer.Color.ToFColor(false);
		}
	}
	return DefaultColor.ToFColor(false);
}