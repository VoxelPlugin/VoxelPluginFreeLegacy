// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelData.h"

namespace FVoxelDataUtilities
{
	template<typename TData>
	struct TBilinearInterpolatedData
	{
		const TData& Data;
		
		explicit TBilinearInterpolatedData(const TData& Data) : Data(Data) {}
		
		v_flt GetValue(v_flt X, v_flt Y, v_flt Z, int32 LOD) const
		{
			const int32 MinX = FMath::FloorToInt(X);
			const int32 MinY = FMath::FloorToInt(Y);
			const int32 MinZ = FMath::FloorToInt(Z);

			const int32 MaxX = FMath::CeilToInt(X);
			const int32 MaxY = FMath::CeilToInt(Y);
			const int32 MaxZ = FMath::CeilToInt(Z);

			const v_flt AlphaX = X - MinX;
			const v_flt AlphaY = Y - MinY;
			const v_flt AlphaZ = Z - MinZ;

			return FVoxelUtilities::TrilinearInterpolation<v_flt>(
				Data.GetValue(MinX, MinY, MinZ, LOD).ToFloat(),
				Data.GetValue(MaxX, MinY, MinZ, LOD).ToFloat(),
				Data.GetValue(MinX, MaxY, MinZ, LOD).ToFloat(),
				Data.GetValue(MaxX, MaxY, MinZ, LOD).ToFloat(),
				Data.GetValue(MinX, MinY, MaxZ, LOD).ToFloat(),
				Data.GetValue(MaxX, MinY, MaxZ, LOD).ToFloat(),
				Data.GetValue(MinX, MaxY, MaxZ, LOD).ToFloat(),
				Data.GetValue(MaxX, MaxY, MaxZ, LOD).ToFloat(),
				AlphaX,
				AlphaY,
				AlphaZ);
		}
		template<typename T>
		v_flt GetValue(const T& P, int32 LOD) const
		{
			return GetValue(P.X, P.Y, P.Z, LOD);
		}
	};

	template<typename TData>
	inline TBilinearInterpolatedData<TData> MakeBilinearInterpolatedData(const TData& Data)
	{
		return TBilinearInterpolatedData<TData>(Data);
	}

	template<typename TData>
	struct TFloatData
	{
		const TData& Data;
		
		explicit TFloatData(const TData& Data) : Data(Data) {}
		
		float GetValue(int32 X, int32 Y, int32 Z, int32 LOD) const
		{
			return Data.GetValue(X, Y, Z, LOD).ToFloat();
		}
		float GetValue(const FIntVector& P, int32 LOD) const
		{
			return GetValue(P.X, P.Y, P.Z, LOD);
		}
	};

	template<typename TData>
	inline TFloatData<TData> MakeFloatData(const TData& Data)
	{
		return TFloatData<TData>(Data);
	}
	
	/**
	 * Requires read lock in FIntBox(Position - Offset, Position + Offset + 1)
	 */
	template<typename T, typename TData>
	inline FVector GetGradientFromGetValue(const TData& Data, T X, T Y, T Z, int32 LOD, T Offset = 1)
	{
		const double MinX = Data.GetValue(X - Offset, Y, Z, LOD);
		const double MaxX = Data.GetValue(X + Offset, Y, Z, LOD);
		const double MinY = Data.GetValue(X, Y - Offset, Z, LOD);
		const double MaxY = Data.GetValue(X, Y + Offset, Z, LOD);
		const double MinZ = Data.GetValue(X, Y, Z - Offset, LOD);
		const double MaxZ = Data.GetValue(X, Y, Z + Offset, LOD);
		return FVector(MaxX - MinX, MaxY - MinY, MaxZ - MinZ).GetSafeNormal();
	}

	template<typename T, typename TData>
	inline FVector GetGradientFromGetFloatValue(TData& Data, T X, T Y, T Z, int32 LOD, T Offset = 1)
	{
		// Force offset to 1 as we don't have data any further outside of the generator
		const auto Fallback = [&]() { return GetGradientFromGetValue<T>(MakeBilinearInterpolatedData(Data), X, Y, Z, LOD, 1); };
		bool bIsGeneratorValue = true;
		
		const double MinX = Data.GetFloatValue(X - Offset, Y, Z, LOD, &bIsGeneratorValue);
		if (!bIsGeneratorValue) return Fallback();
		const double MaxX = Data.GetFloatValue(X + Offset, Y, Z, LOD, &bIsGeneratorValue);
		if (!bIsGeneratorValue) return Fallback();
		const double MinY = Data.GetFloatValue(X, Y - Offset, Z, LOD, &bIsGeneratorValue);
		if (!bIsGeneratorValue) return Fallback();
		const double MaxY = Data.GetFloatValue(X, Y + Offset, Z, LOD, &bIsGeneratorValue);
		if (!bIsGeneratorValue) return Fallback();
		const double MinZ = Data.GetFloatValue(X, Y, Z - Offset, LOD, &bIsGeneratorValue);
		if (!bIsGeneratorValue) return Fallback();
		const double MaxZ = Data.GetFloatValue(X, Y, Z + Offset, LOD, &bIsGeneratorValue);
		if (!bIsGeneratorValue) return Fallback();
		
		return FVector(MaxX - MinX, MaxY - MinY, MaxZ - MinZ).GetSafeNormal();
	}
	
	template<typename T, typename F>
	inline void IterateDirtyDataInBounds(const FVoxelData& Data, const FIntBox& Bounds, F Lambda)
	{
		FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](const FVoxelDataOctreeLeaf& Leaf)
		{
			if (Leaf.GetData<T>().IsDirty())
			{
				const FIntBox LeafBounds = Leaf.GetBounds();
				auto* RESTRICT DataPtr = Leaf.GetData<T>().GetDataPtr();
				LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
				{
					const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
					const T& Value = DataPtr[Index];
					Lambda(X, Y, Z, Value);
				});
			}
		});
	}
}