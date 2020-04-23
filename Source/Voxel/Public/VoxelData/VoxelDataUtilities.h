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
			auto& DataHolder = Leaf.GetData<T>();
			if (DataHolder.IsDirty())
			{
				const FIntBox LeafBounds = Leaf.GetBounds();
				if (DataHolder.IsSingleValue())
				{
					const T Value = DataHolder.GetSingleValue();
					LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
					{
						Lambda(X, Y, Z, Value);
					});
				}
				else
				{
					auto* RESTRICT DataPtr = DataHolder.GetDataPtr();
					check(DataPtr);
					LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
					{
						const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
						const T& Value = DataPtr[Index];
						Lambda(X, Y, Z, Value);
					});
				}
			}
		});
	}

	template<typename T>
	inline void ClearData(FVoxelData& Data)
	{
		FVoxelOctreeUtilities::IterateAllLeaves(Data.GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
		{
			ensureThreadSafe(Leaf.IsLockedForWrite());
			Leaf.GetData<T>().ClearData(Data);
		});
	}
	template<typename T>
	inline bool HasData(FVoxelData& Data)
	{
		return !FVoxelOctreeUtilities::IterateLeavesInBoundsEarlyExit(Data.GetOctree(), FIntBox::Infinite, [](FVoxelDataOctreeLeaf& Leaf)
		{
			ensureThreadSafe(Leaf.IsLockedForRead());
			return !Leaf.GetData<T>().GetDataPtr() && !Leaf.GetData<T>().IsSingleValue();
		});
	}
	template<typename T>
	inline bool CheckIfSameAsGenerator(const FVoxelData& Data, FVoxelDataOctreeLeaf& Leaf)
	{
		VOXEL_SLOW_FUNCTION_COUNTER();
		
		auto& DataHolder = Leaf.GetData<T>();

		if (!ensure(DataHolder.IsDirty())) return false;
		
		const FIntVector Min = Leaf.GetMin();
		if (DataHolder.IsSingleValue())
		{
			// Note: still check this case, as else we end up with the save file being big because of single values!
			// 1024 x 1024 x 1024 world -> 32k possible single values! Ends up being a lot as you store 14 bytes per value
			const T SingleValue = DataHolder.GetSingleValue();
			for (int32 X = 0; X < DATA_CHUNK_SIZE; X++)
			{
				for (int32 Y = 0; Y < DATA_CHUNK_SIZE; Y++)
				{
					for (int32 Z = 0; Z < DATA_CHUNK_SIZE; Z++)
					{
						const T GeneratorValue = Leaf.GetFromGeneratorAndAssets<T>(*Data.WorldGenerator, Min.X + X, Min.Y + Y, Min.Z + Z, 0);
						if (SingleValue != GeneratorValue)
						{
							return false;
						}
					}
				}
			}
		}
		else
		{
			T* RESTRICT const DataPtr = DataHolder.GetDataPtr();
			check(DataPtr);
			for (int32 X = 0; X < DATA_CHUNK_SIZE; X++)
			{
				for (int32 Y = 0; Y < DATA_CHUNK_SIZE; Y++)
				{
					for (int32 Z = 0; Z < DATA_CHUNK_SIZE; Z++)
					{
						const T GeneratorValue = Leaf.GetFromGeneratorAndAssets<T>(*Data.WorldGenerator, Min.X + X, Min.Y + Y, Min.Z + Z, 0);
						const T Value = DataPtr[FVoxelDataOctreeUtilities::IndexFromCoordinates(X, Y, Z)];
						if (Value != GeneratorValue)
						{
							return false;
						}
					}
				}
			}
		}

		DataHolder.SetDirtyInternal(false, Data);
		return true;
	}

	template<typename T>
	inline void SetEntireDataAsDirtyAndCopyFrom(const FVoxelData& SourceData, FVoxelData& DestData)
	{
		FVoxelOctreeUtilities::IterateEntireTree(DestData.GetOctree(), [&](FVoxelDataOctreeBase& Tree)
		{
			if (Tree.IsLeaf())
			{
				FVoxelDataOctreeLeaf& Leaf = Tree.AsLeaf();
				const FVoxelDataOctreeBase& SourceBottomNode = FVoxelOctreeUtilities::GetBottomNode(SourceData.GetOctree(), Leaf.Position.X, Leaf.Position.Y, Leaf.Position.Z);
				const FVoxelDataOctreeLeaf* SourceLeaf = SourceBottomNode.IsLeaf() ? &SourceBottomNode.AsLeaf() : nullptr;
			
				TVoxelDataOctreeLeafData<T>& DataHolder = Leaf.GetData<T>();

				const auto CopyFromGenerator = [&]()
				{
					DataHolder.CreateDataPtr(DestData);
					TVoxelQueryZone<T> QueryZone(Leaf.GetBounds(), DataHolder.GetDataPtr());
					SourceBottomNode.GetFromGeneratorAndAssets(*SourceData.WorldGenerator, QueryZone, 0); // Note: make sure to use the source world generator!
					DataHolder.TryCompressToSingleValue(DestData); // To save memory
				};
				
				if (SourceLeaf)
				{
					const TVoxelDataOctreeLeafData<T>& SourceDataHolder = SourceLeaf->GetData<T>();
					if (SourceDataHolder.IsSingleValue())
					{
						DataHolder.SetSingleValue(SourceDataHolder.GetSingleValue());
					}
					else if (auto* SrcDataPtr = SourceDataHolder.GetDataPtr())
					{
						DataHolder.CreateDataPtr(DestData);
						FMemory::Memcpy(DataHolder.GetDataPtr(), SrcDataPtr, VOXELS_PER_DATA_CHUNK * sizeof(T));
					}
					else
					{
						CopyFromGenerator();
					}
				}
				else
				{
					CopyFromGenerator();
				}
				
				DataHolder.SetDirty(DestData);
			}
			else
			{
				auto& Parent = Tree.AsParent();
				if (!Parent.HasChildren())
				{
					Parent.CreateChildren();
				}
			}
		});
	}
	template<typename T>
	inline void CopyDirtyChunksFrom(const FVoxelData& SourceData, FVoxelData& DestData)
	{
		FVoxelOctreeUtilities::IterateAllLeaves(SourceData.GetOctree(), [&](FVoxelDataOctreeLeaf& SourceLeaf)
		{
			const TVoxelDataOctreeLeafData<T>& SourceDataHolder = SourceLeaf.GetData<T>();

			if (!SourceDataHolder.IsDirty())
			{
				return;
			}

			FVoxelDataOctreeLeaf& DestLeaf = *FVoxelOctreeUtilities::GetLeaf<EVoxelOctreeLeafQuery::CreateIfNull>(DestData.GetOctree(), SourceLeaf.Position.X, SourceLeaf.Position.Y, SourceLeaf.Position.Z);
			TVoxelDataOctreeLeafData<T>& DestDataHolder = DestLeaf.GetData<T>();

			if (SourceDataHolder.IsSingleValue())
			{
				DestDataHolder.SetSingleValue(SourceDataHolder.GetSingleValue());
			}
			else if (auto* SrcDataPtr = SourceDataHolder.GetDataPtr())
			{
				DestDataHolder.CreateDataPtr(DestData);
				FMemory::Memcpy(DestDataHolder.GetDataPtr(), SrcDataPtr, VOXELS_PER_DATA_CHUNK * sizeof(T));
			}
		});
	}

	// Note: will set the entire Data as dirty!
	template<typename T>
	inline void OverrideWorldGeneratorValue(FVoxelData& Data, T Value)
	{
		FVoxelOctreeUtilities::IterateEntireTree(Data.GetOctree(), [&](FVoxelDataOctreeBase& Tree)
		{
			if (Tree.IsLeaf())
			{
				ensureThreadSafe(Tree.IsLockedForWrite());
			
				FVoxelDataOctreeLeaf& Leaf = Tree.AsLeaf();
				TVoxelDataOctreeLeafData<T>& DataHolder = Leaf.GetData<T>();

				if (!DataHolder.IsDirty())
				{
					DataHolder.ClearData(Data);
					DataHolder.SetSingleValue(Value);
					DataHolder.SetDirty(Data);
				}
			}
			else
			{
				auto& Parent = Tree.AsParent();
				if (!Parent.HasChildren())
				{
					Parent.CreateChildren();
				}
			}
		});
	}
}