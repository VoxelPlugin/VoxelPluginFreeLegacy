// Copyright 2018 Phyronnaz

#include "VoxelWorldGenerators/EmptyWorldGenerator.h"
#include "VoxelPrivate.h"

TSharedRef<FVoxelWorldGeneratorInstance> UEmptyWorldGenerator::GetWorldGenerator()
{
	return MakeShareable(new FEmptyWorldGeneratorInstance());
}

////////////////////////////////////////////////////////////////////////////////

void FEmptyWorldGeneratorInstance::GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const
{
	for (int K = 0; K < Size.Z; K++)
	{
		for (int J = 0; J < Size.Y; J++)
		{
			for (int I = 0; I < Size.X; I++)
			{
				const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);
				if (Values)
				{
					Values[Index] = 1;
				}
				if (Materials)
				{
					Materials[Index] = FVoxelMaterial();
				}
				if (VoxelTypes)
				{
					VoxelTypes[Index] = FVoxelType::IgnoreAll();
				}
			}
		}
	}
}

bool FEmptyWorldGeneratorInstance::IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	return true;
}
