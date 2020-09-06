// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAssets/VoxelDataAssetData.h"

FORCEINLINE FVoxelValue FVoxelDataAssetData::GetValue(int32 X, int32 Y, int32 Z, FVoxelValue DefaultValue) const
{
	if (IsValidIndex(X, Y, Z))
	{
		return GetValueUnsafe(X, Y, Z);
	}
	else
	{
		return DefaultValue;
	}
}

FORCEINLINE FVoxelMaterial FVoxelDataAssetData::GetMaterial(int32 X, int32 Y, int32 Z) const
{
	if (IsValidIndex(X, Y, Z))
	{
		return GetMaterialUnsafe(X, Y, Z);
	}
	else
	{
		return FVoxelMaterial::Default();
	}
}

inline float FVoxelDataAssetData::GetInterpolatedValue(float X, float Y, float Z, FVoxelValue DefaultValue, float Tolerance) const
{
	const int32 RoundedX = FMath::RoundToInt(X);
	const int32 RoundedY = FMath::RoundToInt(Y);
	const int32 RoundedZ = FMath::RoundToInt(Z);
	if (FMath::IsNearlyEqual(X, RoundedX, Tolerance) && 
		FMath::IsNearlyEqual(Y, RoundedY, Tolerance) && 
		FMath::IsNearlyEqual(Z, RoundedZ, Tolerance))
	{
		return GetValue(RoundedX, RoundedY, RoundedZ, DefaultValue).ToFloat();
	}
	
	const int32 MinX = FMath::FloorToInt(X);
	const int32 MinY = FMath::FloorToInt(Y);
	const int32 MinZ = FMath::FloorToInt(Z);

	const int32 MaxX = FMath::CeilToInt(X);
	const int32 MaxY = FMath::CeilToInt(Y);
	const int32 MaxZ = FMath::CeilToInt(Z);

	const float AlphaX = X - MinX;
	const float AlphaY = Y - MinY;
	const float AlphaZ = Z - MinZ;

	return FVoxelUtilities::TrilinearInterpolation<float>(
		GetValue(MinX, MinY, MinZ, DefaultValue).ToFloat(),
		GetValue(MaxX, MinY, MinZ, DefaultValue).ToFloat(),
		GetValue(MinX, MaxY, MinZ, DefaultValue).ToFloat(),
		GetValue(MaxX, MaxY, MinZ, DefaultValue).ToFloat(),
		GetValue(MinX, MinY, MaxZ, DefaultValue).ToFloat(),
		GetValue(MaxX, MinY, MaxZ, DefaultValue).ToFloat(),
		GetValue(MinX, MaxY, MaxZ, DefaultValue).ToFloat(),
		GetValue(MaxX, MaxY, MaxZ, DefaultValue).ToFloat(),
		AlphaX,
		AlphaY,
		AlphaZ);
}

inline FVoxelMaterial FVoxelDataAssetData::GetInterpolatedMaterial(float X, float Y, float Z, float Tolerance) const
{
	const int32 RoundedX = FMath::RoundToInt(X);
	const int32 RoundedY = FMath::RoundToInt(Y);
	const int32 RoundedZ = FMath::RoundToInt(Z);
	if (FMath::IsNearlyEqual(X, RoundedX, Tolerance) && 
		FMath::IsNearlyEqual(Y, RoundedY, Tolerance) && 
		FMath::IsNearlyEqual(Z, RoundedZ, Tolerance))
	{
		return GetMaterial(RoundedX, RoundedY, RoundedZ);
	}

	// Note: might get better results by interpolating the material colors/UVs

	X = FMath::Clamp<float>(X, 0, Size.X - 1);
	Y = FMath::Clamp<float>(Y, 0, Size.Y - 1);
	Z = FMath::Clamp<float>(Z, 0, Size.Z - 1);

	auto* RESTRICT const ValuesPtr = Values.GetData();
	auto* RESTRICT const MaterialsPtr = Materials.GetData();
	const int32 MinX = FMath::FloorToInt(X);
	const int32 MinY = FMath::FloorToInt(Y);
	const int32 MinZ = FMath::FloorToInt(Z);
	const int32 MaxX = FMath::CeilToInt(X);
	const int32 MaxY = FMath::CeilToInt(Y);
	const int32 MaxZ = FMath::CeilToInt(Z);
	for (int32 ItX = MinX; ItX <= MaxX; ItX++)
	{
		for (int32 ItY = MinY; ItY <= MaxY; ItY++)
		{
			for (int32 ItZ = MinZ; ItZ <= MaxZ; ItZ++)
			{
				checkVoxelSlow(IsValidIndex(ItX, ItY, ItZ));
				const int32 Index = GetIndex(ItX, ItY, ItZ);
				checkVoxelSlow(Values.IsValidIndex(Index));
				checkVoxelSlow(Materials.IsValidIndex(Index));
				if (ValuesPtr[Index].IsEmpty()) continue;
				return MaterialsPtr[Index];
			}
		}
	}
	return MaterialsPtr[GetIndex(MinX, MinY, MinZ)];
}