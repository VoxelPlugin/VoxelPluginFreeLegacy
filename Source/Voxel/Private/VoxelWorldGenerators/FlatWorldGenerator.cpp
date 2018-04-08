// Copyright 2018 Phyronnaz

#include "VoxelWorldGenerators/FlatWorldGenerator.h"

FFlatWorldGeneratorInstance::FFlatWorldGeneratorInstance(int TerrainHeight, float FadeHeight, const TArray<FFlatWorldLayer>& TerrainLayers)
	: TerrainHeight(TerrainHeight)
	, FadeHeight(FadeHeight)
	, TerrainLayers(TerrainLayers)
{

}

void FFlatWorldGeneratorInstance::GetValuesAndMaterialsAndVoxelTypes(float Values[], FVoxelMaterial Materials[], FVoxelType VoxelTypes[], const FIntVector& Start, const FIntVector& StartIndex, const int Step, const FIntVector& Size, const FIntVector& ArraySize) const
{
	for (int K = 0; K < Size.Z; K++)
	{
		const int Z = Start.Z + K * Step;
		const float Value = (Z >= TerrainHeight) ? 1 : -1;
		FVoxelMaterial Material = FVoxelMaterial();
		if (Materials && TerrainLayers.Num())
		{
			if (Z < TerrainLayers[0].Start)
			{
				Material = FVoxelMaterial(TerrainLayers[0].Material, TerrainLayers[0].Material, 255, 0);
			}
			else
			{
				const int LastIndex = TerrainLayers.Num() - 1;
				if (LastIndex >= 0 && Z >= TerrainLayers[LastIndex].Start)
				{
					Material = FVoxelMaterial(TerrainLayers[LastIndex].Material, TerrainLayers[LastIndex].Material, (LastIndex % 2 == 0) ? 255 : 0, 0);
				}
				else
				{
					for (int i = 0; i < TerrainLayers.Num() - 1; i++)
					{
						if (TerrainLayers[i].Start <= Z && Z < TerrainLayers[i + 1].Start)
						{
							const uint8 Alpha = FMath::Clamp<int>(255 * (TerrainLayers[i + 1].Start - 1 - Z) / FadeHeight, 0, 255);

							// Alternate first material to avoid issues with alpha smoothing
							if (i % 2 == 0)
							{
								Material = FVoxelMaterial(TerrainLayers[i + 1].Material, TerrainLayers[i].Material, Alpha, 0);
							}
							else
							{
								Material = FVoxelMaterial(TerrainLayers[i].Material, TerrainLayers[i + 1].Material, 255 - Alpha, 0);
							}
						}
					}
				}
			}
		}
		for (int J = 0; J < Size.Y; J++)
		{
			const int Y = Start.Y + J * Step;
			for (int I = 0; I < Size.X; I++)
			{
				const int X = Start.X + I * Step;
				const int Index = (StartIndex.X + I) + ArraySize.X * (StartIndex.Y + J) + ArraySize.X * ArraySize.Y * (StartIndex.Z + K);

				if (Values)
				{
					Values[Index] = Value;
				}
				if (Materials)
				{
					Materials[Index] = Material;
				}
				if (VoxelTypes)
				{
					VoxelTypes[Index] = FVoxelType(Z >= TerrainHeight ? EVoxelValueType::IgnoreValue : EVoxelValueType::UseValue, EVoxelMaterialType::UseMaterial);
				}
			}
		}
	}
}

bool FFlatWorldGeneratorInstance::IsEmpty(const FIntVector& Start, const int Step, const FIntVector& Size) const
{
	return Start.Z + (Size.Z - 1) * Step < TerrainHeight - 1 || TerrainHeight + 1 < Start.Z;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UFlatWorldGenerator::UFlatWorldGenerator()
	: TerrainHeight(0)
	, FadeHeight(4)
{

}


TSharedRef<FVoxelWorldGeneratorInstance> UFlatWorldGenerator::GetWorldGenerator()
{
	TArray<FFlatWorldLayer> SortedTerrainLayers = TerrainLayers;
	SortedTerrainLayers.Sort([](const FFlatWorldLayer& Left, const FFlatWorldLayer& Right) { return Left.Start < Right.Start; });
	return MakeShareable(new FFlatWorldGeneratorInstance(TerrainHeight, FadeHeight, SortedTerrainLayers));
}