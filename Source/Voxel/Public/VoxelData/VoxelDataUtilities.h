// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelData.h"

namespace FVoxelDataUtilities
{
	/**
	 * Won't work if a setter is called in the map bounds after the map creation
	 */
	struct MapAccelerator
	{
	private:
		struct FLastMapQuery
		{
			FIntVector Position;
			const FVoxelDataOctree** Chunk = nullptr;
			bool bValid = false;
		};

		FVoxelMap Map;
		FLastMapQuery LastQuery;
		const FVoxelData& Data;

	public:
		MapAccelerator(const FIntBox& Bounds, const FVoxelData* InData)
			: Data(*InData)
		{
			Data.GetOctree()->GetMap(Bounds, Map);
			Map.Compact();
		}

		inline void GetValueAndMaterial(int32 X, int32 Y, int32 Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int32 QueryLOD)
		{
			FIntVector Position = FVoxelUtilities::DivideFloor(FIntVector(X, Y, Z), VOXEL_CELL_SIZE);
			if (!LastQuery.bValid || LastQuery.Position != Position)
			{
				LastQuery.Position = Position;
				LastQuery.Chunk = Map.Find(Position);
				LastQuery.bValid = true;
			}
			if (LastQuery.Chunk)
			{
				(*LastQuery.Chunk)->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD);
			}
			else
			{
				Data.GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD);
			}
		}
		inline void GetValueAndMaterial(int32 X, int32 Y, int32 Z, FVoxelValue& OutValue, FVoxelMaterial& OutMaterial, int32 QueryLOD) { GetValueAndMaterial(X, Y, Z, &OutValue, &OutMaterial, QueryLOD); }

		inline FVoxelValue GetValue(int32 X, int32 Y, int32 Z, int32 QueryLOD)
		{
			FVoxelValue Value;
			GetValueAndMaterial(X, Y, Z, &Value, nullptr, QueryLOD);
			return Value;
		}
		inline FVoxelValue GetValue(const FIntVector& P, int32 QueryLOD) { return GetValue(P.X, P.Y, P.Z, QueryLOD); }

		inline FVoxelMaterial GetMaterial(int32 X, int32 Y, int32 Z, int32 QueryLOD)
		{
			FVoxelMaterial Material;
			GetValueAndMaterial(X, Y, Z, nullptr, &Material, QueryLOD);
			return Material;
		}
		inline FVoxelMaterial GetMaterial(const FIntVector& P, int32 QueryLOD) { return GetMaterial(P.X, P.Y, P.Z, QueryLOD); }
	};

	/**
	 * Works with setters
	 */
	struct LastOctreeAccelerator
	{
	private:
		FVoxelDataOctree* LastOctree = nullptr;
		const FVoxelData& Data;

	public:
		LastOctreeAccelerator(const FVoxelData& Data)
			: Data(Data)
		{
		}

		inline void GetValueAndMaterial(int32 X, int32 Y, int32 Z, FVoxelValue* OutValue, FVoxelMaterial* OutMaterial, int32 QueryLOD)
		{
			if (UNLIKELY(!Data.IsInWorld(X, Y, Z)))
			{
				Data.WorldGenerator->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD, FVoxelPlaceableItemHolder());
			}
			else
			{
				if (UNLIKELY(!LastOctree || !LastOctree->IsInOctree(X, Y, Z) || !LastOctree->IsLeaf()))
				{
					LastOctree = Data.GetOctree()->GetLeaf(X, Y, Z);
				}
				LastOctree->GetValueAndMaterial(X, Y, Z, OutValue, OutMaterial, QueryLOD);
			}
		}
		template<typename T>
		void SetValueOrMaterial(int32 X, int32 Y, int32 Z, const T& Value)
		{
			if (UNLIKELY(!Data.IsInWorld(X, Y, Z)))
			{
				return;
			}
			if (UNLIKELY(!LastOctree || !LastOctree->IsInOctree(X, Y, Z) || !LastOctree->IsLeaf()))
			{
				LastOctree = Data.GetOctree()->GetLeaf(X, Y, Z);
			}
			LastOctree->SetValueOrMaterial<T>(X, Y, Z, Value);
		}

		inline void GetValueAndMaterial(int32 X, int32 Y, int32 Z, FVoxelValue& OutValue, FVoxelMaterial& OutMaterial, int32 QueryLOD) { GetValueAndMaterial(X, Y, Z, &OutValue, &OutMaterial, QueryLOD); }
		inline void GetValueAndMaterial(const FIntVector& P, FVoxelValue& OutValue, FVoxelMaterial& OutMaterial, int32 QueryLOD) { GetValueAndMaterial(P.X, P.Y, P.Z, &OutValue, &OutMaterial, QueryLOD); }

		inline FVoxelValue GetValue(int32 X, int32 Y, int32 Z, int32 QueryLOD) { FVoxelValue Value; GetValueAndMaterial(X, Y, Z, &Value, nullptr, QueryLOD); return Value; }
		inline FVoxelValue GetValue(const FIntVector& P, int32 QueryLOD) { return GetValue(P.X, P.Y, P.Z, QueryLOD); }

		inline FVoxelMaterial GetMaterial(int32 X, int32 Y, int32 Z, int32 QueryLOD) { FVoxelMaterial Material; GetValueAndMaterial(X, Y, Z, nullptr, &Material, QueryLOD); return Material; }
		inline FVoxelMaterial GetMaterial(const FIntVector& P, int32 QueryLOD) { return GetMaterial(P.X, P.Y, P.Z, QueryLOD); }

		inline void SetValue(int32 X, int32 Y, int32 Z, FVoxelValue Value) { SetValueOrMaterial<FVoxelValue>(X, Y, Z, Value); }
		inline void SetValue(const FIntVector& P, FVoxelValue Value) { SetValueOrMaterial<FVoxelValue>(P.X, P.Y, P.Z, Value); }

		inline void SetMaterial(int32 X, int32 Y, int32 Z, FVoxelMaterial Material) { SetValueOrMaterial<FVoxelMaterial>(X, Y, Z, Material); }
		inline void SetMaterial(const FIntVector& P, FVoxelMaterial Material) { SetValueOrMaterial<FVoxelMaterial>(P.X, P.Y, P.Z, Material); }
	};
	
	/**
	 * Requires read lock in FIntBox(Position - Offset, Position + Offset + 1)
	 */
	template<typename T>
	inline FVector GetGradient(T& Data, int32 X, int32 Y, int32 Z, int32 QueryLOD, int32 Offset = 1)
	{
		FVector Gradient;
		Gradient.X = Data.GetValue(X + Offset, Y, Z, QueryLOD).ToFloat() - Data.GetValue(X - Offset, Y, Z, QueryLOD).ToFloat();
		Gradient.Y = Data.GetValue(X, Y + Offset, Z, QueryLOD).ToFloat() - Data.GetValue(X, Y - Offset, Z, QueryLOD).ToFloat();
		Gradient.Z = Data.GetValue(X, Y, Z + Offset, QueryLOD).ToFloat() - Data.GetValue(X, Y, Z - Offset, QueryLOD).ToFloat();
		return Gradient.GetSafeNormal();
	}
	template<typename T>
	inline FVector GetGradient(T& Data, const FIntVector& P, int32 QueryLOD)
	{
		return GetGradient(Data, P.X, P.Y, P.Z, QueryLOD);
	}
}