// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelMaterial.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"

struct VOXEL_API FVoxelContext
{
	const FVoxelPlaceableItemHolder& ItemHolder;
	const int LOD;
	int32 X;
	int32 Y;
	int32 Z;

	FVoxelContext(const FVoxelPlaceableItemHolder& ItemHolder, int LOD, int X = 0, int Y = 0, int Z = 0)
		: ItemHolder(ItemHolder)
		, LOD(LOD)
		, X(X)
		, Y(Y)
		, Z(Z)
	{
	}

	FVoxelContext(int X, int Y, int Z)
		: ItemHolder(StaticItem)
		, LOD(0)
		, X(X)
		, Y(Y)
		, Z(Z)
	{
	}

	FVoxelContext()
		: ItemHolder(StaticItem)
		, LOD(0)
		, X(0)
		, Y(0)
		, Z(0)
	{
	}

private:
	static const FVoxelPlaceableItemHolder StaticItem;
};

namespace FVoxelNodeFunctions
{
	inline void GetDoubleIndex(const FVoxelMaterial& Material, int& OutA, int& OutB, float& OutBlend)
	{
		OutA = Material.GetIndexA();
		OutB = Material.GetIndexB();
		OutBlend = Material.GetBlend() / 255.999f;
	}

	inline void GetColor(const FVoxelMaterial& Material, float& OutR, float& OutG, float& OutB, float& OutA)
	{
		FLinearColor Color = Material.GetLinearColor();
		OutR = Color.R;
		OutG = Color.G;
		OutB = Color.B;
		OutA = Color.A;
	}
	// For back compatibility
	inline void GetColor(const FVoxelMaterial& Material, float& OutR, float& OutG, float& OutB)
	{
		float Dummy;
		GetColor(Material, OutR, OutG, OutB, Dummy);
	}

	inline float VectorLength(float X, float Y, float Z)
	{
		return FMath::Sqrt(X*X + Y*Y + Z*Z);
	}

	inline void VectorRotateAngleAxis(float X, float Y, float Z, float AxisX, float AxisY, float AxisZ, float Angle, float& OutX, float& OutY, float& OutZ)
	{
		// taken from FVector; not using directly to keep from allocating an fvector
		float S, C;
		FMath::SinCos(&S, &C, FMath::DegreesToRadians(Angle));

		const float XX = AxisX * AxisX;
		const float YY = AxisY * AxisY;
		const float ZZ = AxisZ * AxisZ;

		const float XY = AxisX * AxisY;
		const float YZ = AxisY * AxisZ;
		const float ZX = AxisZ * AxisX;

		const float XS = AxisX * S;
		const float YS = AxisY * S;
		const float ZS = AxisZ * S;

		const float OMC = 1.f - C;

		OutX = (OMC * XX + C) * X + (OMC * XY - ZS) * Y + (OMC * ZX + YS) * Z;
		OutY = (OMC * XY + ZS) * X + (OMC * YY + C) * Y + (OMC * YZ - XS) * Z;
		OutZ = (OMC * ZX - YS) * X + (OMC * YZ + XS) * Y + (OMC * ZZ + C) * Z;
	}
	inline float GetPerlinWormsDistance(const FVoxelPlaceableItemHolder& ItemHolder, const FVector& Position)
	{
		float Dist = 0;
		return Dist;
	}

	inline float Sqrt(float F)
	{
		if (F <= 0)
		{
			return 0;
		}
		else
		{
			return FMath::Sqrt(F);
		}
	}
}
