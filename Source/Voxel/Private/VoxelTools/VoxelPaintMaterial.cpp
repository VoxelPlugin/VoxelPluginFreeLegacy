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

FVoxelPaintMaterial FVoxelPaintMaterial::CreateFiveWayBlend(int32 Channel, float TargetValue, bool bPaintR, bool bPaintG, bool bPaintB, bool bPaintA)
{
	ensure(0 <= Channel && Channel < 5);
	
	FVoxelPaintMaterial Material;
	Material.Type = EVoxelPaintMaterialType::FiveWayBlend;
	Material.FiveWayBlend.Channel = Channel;
	Material.FiveWayBlend.TargetValue = TargetValue;
	Material.FiveWayBlend.bPaintR = bPaintR;
	Material.FiveWayBlend.bPaintG = bPaintG;
	Material.FiveWayBlend.bPaintB = bPaintB;
	Material.FiveWayBlend.bPaintA = bPaintA;
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
			IntColor = FColor(0, 0, 0, 0);
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
	case EVoxelPaintMaterialType::FiveWayBlend:
	{
		if (!ensure(0 <= FiveWayBlend.Channel && FiveWayBlend.Channel < 5))
		{
			return;
		}

		const float TargetValue = Strength > 0 ? FiveWayBlend.TargetValue : 1.f - FiveWayBlend.TargetValue;
		Strength = FMath::Abs(Strength);
		
		const float R = FiveWayBlend.bPaintR ? Material.GetR_AsFloat() : 0.f;
		const float G = FiveWayBlend.bPaintG ? Material.GetG_AsFloat() : 0.f;
		const float B = FiveWayBlend.bPaintB ? Material.GetB_AsFloat() : 0.f;
		const float A = FiveWayBlend.bPaintA ? Material.GetA_AsFloat() : 0.f;

		TStaticArray<float, 5> Strengths = FVoxelUtilities::ConvertRGBAToFiveWayBlendStrengths(FVector4(R, G, B, A));
		
		Strengths[FiveWayBlend.Channel] = FMath::Clamp(FMath::Lerp(Strengths[FiveWayBlend.Channel], TargetValue, Strength), 0.f, 1.f);

		const FVector4 NewColor = FVoxelUtilities::ConvertFiveWayBlendStrengthsToRGBA(Strengths);
		
		const auto CustomFloatToUINT8 = [](float NewValue, float OldValue)
		{
			// Round up if the new value is higher than the previous one, to avoid being stuck
			return FVoxelUtilities::ClampToUINT8(NewValue > OldValue
				? FMath::CeilToInt(NewValue * 255.999f)
				: FMath::FloorToInt(NewValue * 255.999f));
		};

		if (FiveWayBlend.bPaintR) Material.SetR(CustomFloatToUINT8(NewColor.X, R));
		if (FiveWayBlend.bPaintG) Material.SetG(CustomFloatToUINT8(NewColor.Y, G));
		if (FiveWayBlend.bPaintB) Material.SetB(CustomFloatToUINT8(NewColor.Z, B));
		if (FiveWayBlend.bPaintA) Material.SetA(CustomFloatToUINT8(NewColor.W, A));
		
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
		int32 SlotIndex;
		if (Index == Material.GetDoubleIndex_IndexA())
		{
			SlotIndex = 0;
		}
		else if (Index == Material.GetDoubleIndex_IndexB())
		{
			SlotIndex = 1;
		}
		else if (Material.GetDoubleIndex_Blend() == 255)
		{
			Material.SetDoubleIndex_IndexA(Index);
			SlotIndex = 0;
		}
		else if (Material.GetDoubleIndex_Blend() == 0)
		{
			Material.SetDoubleIndex_IndexB(Index);
			SlotIndex = 1;
		}
		else
		{
			// Fill as fast as we can
			SlotIndex = Material.GetDoubleIndex_Blend() < 128 ? 0 : 1;
		}

		Material.SetDoubleIndex_Blend(FVoxelUtilities::ClampToUINT8(Material.GetDoubleIndex_Blend() + 255 * Strength * (SlotIndex == 0 ? -1 : +1)));

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
