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
	TBilinearInterpolatedData<TData> MakeBilinearInterpolatedData(const TData& Data)
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
	TFloatData<TData> MakeFloatData(const TData& Data)
	{
		return TFloatData<TData>(Data);
	}
	
	/**
	 * Requires read lock in FVoxelIntBox(Position - Offset, Position + Offset + 1)
	 */
	template<typename T, typename TData>
	FVector GetGradientFromGetValue(const TData& Data, T X, T Y, T Z, int32 LOD, T Offset = 1);

	template<typename T, typename TData>
	FVector GetGradientFromGetFloatValue(TData& Data, T X, T Y, T Z, int32 LOD, T Offset = 1);

	template<typename T, typename F>
	void IterateDirtyDataInBounds(const FVoxelData& Data, const FVoxelIntBox& Bounds, F Lambda);

	template<typename T>
	void ClearData(FVoxelData& Data);

	template<typename T>
	bool HasData(FVoxelData& Data);

	template<typename T>
	bool CheckIfSameAsGenerator(const FVoxelData& Data, FVoxelDataOctreeLeaf& Leaf);

	template<typename T>
	void SetEntireDataAsDirtyAndCopyFrom(const FVoxelData& SourceData, FVoxelData& DestData);

	template<typename T>
	void CopyDirtyChunksFrom(const FVoxelData& SourceData, FVoxelData& DestData);

	// Note: will set the entire Data as dirty!
	template<typename T>
	void OverrideGeneratorValue(FVoxelData& Data, T Value);

	template<typename T>
	void ScaleWorldData(const FVoxelData& SourceData, FVoxelData& DestData, const FVoxelVector& Scale);
}