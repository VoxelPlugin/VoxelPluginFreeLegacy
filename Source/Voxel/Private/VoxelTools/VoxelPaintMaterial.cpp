// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelPaintMaterial.h"
#include "VoxelMathUtilities.h"

FVoxelPaintMaterial FVoxelPaintMaterial::CreateRGB(FLinearColor Color, bool bPaintR, bool bPaintG, bool bPaintB, bool bPaintA)
{
	FVoxelPaintMaterial Material;
	Material.Type = EVoxelPaintMaterialType::RGB;
	Material.Color.Color = Color;
	Material.Color.bPaintR = bPaintR;
	Material.Color.bPaintG = bPaintG;
	Material.Color.bPaintB = bPaintB;
	Material.Color.bPaintA = bPaintA;
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

FVoxelPaintMaterial FVoxelPaintMaterial::CreateDoubleIndexBlend(uint8 Index)
{
	FVoxelPaintMaterial Material;
	Material.Type = EVoxelPaintMaterialType::DoubleIndexBlend;
	Material.Index = Index;
	return Material;
}

FVoxelPaintMaterial FVoxelPaintMaterial::CreateUV(uint8 Channel, FVector2D UV, bool bPaintU, bool bPaintV)
{
	FVoxelPaintMaterial Material;
	Material.Type = EVoxelPaintMaterialType::UVs;
	Material.UV.Channel = Channel;
	Material.UV.UV = UV;
	Material.UV.bPaintU = bPaintU;
	Material.UV.bPaintV = bPaintV;
	return Material;
}

inline uint8 LerpUINT8_CustomRounding(uint8 A, uint8 B, float Amount)
{
	const float LerpResult = FMath::Lerp<float>(A, B, Amount);
	// Do special rounding to not get stuck, eg Lerp(251, 255, 0.1) = 251
	const int32 RoundedResult = Amount > 0 ? FMath::CeilToInt(LerpResult) : FMath::FloorToInt(LerpResult);
	return FVoxelUtilities::ClampToUINT8(RoundedResult);
}

void FVoxelPaintMaterial::ApplyToMaterial(FVoxelMaterial& Material, float Strength) const
{
	switch (Type)
	{
	case EVoxelPaintMaterialType::RGB:
	{
		FColor IntColor = Color.Color.ToFColor(false);
		if (Strength < 0)
		{
			Strength = -Strength;
			IntColor = FColor::Black;
		}
		if (Color.bPaintR)
		{
			Material.SetR(LerpUINT8_CustomRounding(Material.GetR(), IntColor.R, Strength));
		}
		if (Color.bPaintG)
		{
			Material.SetG(LerpUINT8_CustomRounding(Material.GetG(), IntColor.G, Strength));
		}
		if (Color.bPaintB)
		{
			Material.SetB(LerpUINT8_CustomRounding(Material.GetB(), IntColor.B, Strength));
		}
		if (Color.bPaintA)
		{
			Material.SetA(LerpUINT8_CustomRounding(Material.GetA(), IntColor.A, Strength));
		}
		break;
	}
	case EVoxelPaintMaterialType::SingleIndex:
	{
		Material.SetSingleIndex_Index(Index);
		break;
	}
	case EVoxelPaintMaterialType::DoubleIndexSet:
	{
		if (DoubleIndexSet.bSetIndexA)
		{
			Material.SetDoubleIndex_IndexA(DoubleIndexSet.IndexA);
		}
		if (DoubleIndexSet.bSetIndexB)
		{
			Material.SetDoubleIndex_IndexB(DoubleIndexSet.IndexB);
		}
		if (DoubleIndexSet.bSetBlend)
		{
			Material.SetDoubleIndex_Blend_AsFloat(DoubleIndexSet.Blend);
		}
		break;
	}
	case EVoxelPaintMaterialType::DoubleIndexBlend:
	{
		if (Index != Material.GetDoubleIndex_IndexA() && Index != Material.GetDoubleIndex_IndexB())
		{
			if (Strength <= 0) return;
			if (Material.GetDoubleIndex_Blend() < 128)
			{
				Material.SetDoubleIndex_IndexB(Index);
				Material.SetDoubleIndex_Blend(0);
			}
			else
			{
				Material.SetDoubleIndex_IndexA(Index);
				Material.SetDoubleIndex_Blend(255);
			}
		}

		if (Index == Material.GetDoubleIndex_IndexA())
		{
			Material.SetDoubleIndex_Blend(FVoxelUtilities::ClampToUINT8(Material.GetDoubleIndex_Blend() - 255 * Strength));
		}
		else if (Index == Material.GetDoubleIndex_IndexB())
		{
			Material.SetDoubleIndex_Blend(FVoxelUtilities::ClampToUINT8(Material.GetDoubleIndex_Blend() + 255 * Strength));
		}
		break;
	}
	case EVoxelPaintMaterialType::UVs:
	{
		FVector2D TargetUV = UV.UV;
		if (Strength < 0)
		{
			Strength = -Strength;
			TargetUV = FVector2D(1, 1) - TargetUV;
		}
		if (UV.bPaintU)
		{
			Material.SetU(UV.Channel, LerpUINT8_CustomRounding(Material.GetU(UV.Channel), FVoxelUtilities::FloatToUINT8(TargetUV.X), Strength));
		}
		if (UV.bPaintV)
		{
			Material.SetV(UV.Channel, LerpUINT8_CustomRounding(Material.GetV(UV.Channel), FVoxelUtilities::FloatToUINT8(TargetUV.Y), Strength));
		}
		break;
	}
	default: ensure(false);
	}
}
