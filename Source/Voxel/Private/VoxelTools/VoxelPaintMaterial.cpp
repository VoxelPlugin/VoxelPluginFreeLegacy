// Copyright 2019 Phyronnaz

#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelGlobals.h"
#include "VoxelMathUtilities.h"

FVoxelPaintMaterial FVoxelPaintMaterial::CreateRGB(FLinearColor Color, float Amount, bool bPaintR, bool bPaintG, bool bPaintB, bool bPaintA)
{
	FVoxelPaintMaterial Material;
	Material.Type = EVoxelPaintMaterialType::RGB;
	Material.Color.Color = Color;
	Material.Color.bPaintR = bPaintR;
	Material.Color.bPaintG = bPaintG;
	Material.Color.bPaintB = bPaintB;
	Material.Color.bPaintA = bPaintA;
	Material.Amount = Amount;
	return Material;
}

FVoxelPaintMaterial FVoxelPaintMaterial::CreateSingleIndex(uint8 Index)
{
	FVoxelPaintMaterial Material;
	Material.Type = EVoxelPaintMaterialType::SingleIndex;
	Material.Index = Index;
	return Material;
}

FVoxelPaintMaterial FVoxelPaintMaterial::CreateDoubleIndexSet(uint8 IndexA, uint8 IndexB, float Blend, bool bSetIndexA, bool bSetIndexB, bool bSetBlend)
{
	FVoxelPaintMaterial Material;
	Material.Type = EVoxelPaintMaterialType::DoubleIndexSet;
	Material.DoubleIndexSet.IndexA = IndexA;
	Material.DoubleIndexSet.IndexB = IndexB;
	Material.DoubleIndexSet.Blend = Blend;
	Material.DoubleIndexSet.bSetIndexA = bSetIndexA;
	Material.DoubleIndexSet.bSetIndexB = bSetIndexB;
	Material.DoubleIndexSet.bSetBlend = bSetBlend;
	return Material;
}

FVoxelPaintMaterial FVoxelPaintMaterial::CreateDoubleIndexBlend(uint8 Index, float Amount)
{
	FVoxelPaintMaterial Material;
	Material.Type = EVoxelPaintMaterialType::DoubleIndexBlend;
	Material.Index = Index;
	Material.Amount = Amount;
	return Material;
}

void FVoxelPaintMaterial::ApplyToMaterial(FVoxelMaterial& Material, float AmountMultiplier) const
{
	switch (Type)
	{
	case EVoxelPaintMaterialType::RGB:
	{
		Material.AddColor(Color.Color, Amount * AmountMultiplier, Color.bPaintR, Color.bPaintG, Color.bPaintB, Color.bPaintA);
		break;
	}
	case EVoxelPaintMaterialType::SingleIndex:
	{
		Material.SetIndex(Index);
		break;
	}
	case EVoxelPaintMaterialType::DoubleIndexSet:
	{
		if (DoubleIndexSet.bSetIndexA)
		{
			Material.SetIndexA(DoubleIndexSet.IndexA);
		}
		if (DoubleIndexSet.bSetIndexB)
		{
			Material.SetIndexB(DoubleIndexSet.IndexB);
		}
		if (DoubleIndexSet.bSetBlend)
		{
			Material.SetBlend(FVoxelUtilities::FloatToUINT8(DoubleIndexSet.Blend));
		}
		break;
	}
	case EVoxelPaintMaterialType::DoubleIndexBlend:
	{
		if (Index != Material.GetIndexA() && Index != Material.GetIndexB())
		{
			if (Material.GetBlend() < 128)
			{
				Material.SetIndexB(Index);
				Material.SetBlend(0);
			}
			else
			{
				Material.SetIndexA(Index);
				Material.SetBlend(255);
			}
		}

		if (Index == Material.GetIndexA())
		{
			Material.SetBlend(FVoxelUtilities::LerpUINT8(Material.GetBlend(), 0, Amount));
		}
		else if (Index == Material.GetIndexB())
		{
			Material.SetBlend(FVoxelUtilities::LerpUINT8(Material.GetBlend(), 255, Amount));
		}
		break;
	}
	default:
		check(false);
	}
}
