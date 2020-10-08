// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelDataUtilities.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelFeedbackContext.h"

template<typename T, typename TData>
FVector FVoxelDataUtilities::GetGradientFromGetValue(const TData& Data, T X, T Y, T Z, int32 LOD, T Offset)
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
FVector FVoxelDataUtilities::GetGradientFromGetFloatValue(TData& Data, T X, T Y, T Z, int32 LOD, T Offset)
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
void FVoxelDataUtilities::IterateDirtyDataInBounds(const FVoxelData& Data, const FVoxelIntBox& Bounds, F Lambda)
{
	FVoxelOctreeUtilities::IterateLeavesInBounds(Data.GetOctree(), Bounds, [&](const FVoxelDataOctreeLeaf& Leaf)
	{
		auto& DataHolder = Leaf.GetData<T>();
		if (DataHolder.IsDirty())
		{
			const FVoxelIntBox LeafBounds = Leaf.GetBounds();
			LeafBounds.Iterate([&](int32 X, int32 Y, int32 Z)
			{
				const FVoxelCellIndex Index = FVoxelDataOctreeUtilities::IndexFromGlobalCoordinates(LeafBounds.Min, X, Y, Z);
				const T& Value = DataHolder.Get(Index);
				Lambda(X, Y, Z, Value);
			});
		}
	});
}

template<typename T>
void FVoxelDataUtilities::ClearData(FVoxelData& Data)
{
	FVoxelOctreeUtilities::IterateAllLeaves(Data.GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForWrite());
		Leaf.GetData<T>().ClearData(Data);
	});
}

template<typename T>
bool FVoxelDataUtilities::HasData(FVoxelData& Data)
{
	return !FVoxelOctreeUtilities::IterateLeavesInBoundsEarlyExit(Data.GetOctree(), FVoxelIntBox::Infinite, [](FVoxelDataOctreeLeaf& Leaf)
	{
		ensureThreadSafe(Leaf.IsLockedForRead());
		return !Leaf.GetData<T>().HasData();
	});
}

template<typename T>
bool FVoxelDataUtilities::CheckIfSameAsGenerator(const FVoxelData& Data, FVoxelDataOctreeLeaf& Leaf)
{
	VOXEL_SLOW_FUNCTION_COUNTER();

	auto& DataHolder = Leaf.GetData<T>();

	if (!ensure(DataHolder.IsDirty())) return false;

	const FIntVector Min = Leaf.GetMin();
	
	// Note: check if single value too, as else we end up with the save file being big because of single values!
	// 1024 x 1024 x 1024 world -> 32k possible single values! Ends up being a lot as you store 14 bytes per value
	for (int32 X = 0; X < DATA_CHUNK_SIZE; X++)
	{
		for (int32 Y = 0; Y < DATA_CHUNK_SIZE; Y++)
		{
			for (int32 Z = 0; Z < DATA_CHUNK_SIZE; Z++)
			{
				const T GeneratorValue = Leaf.GetFromGeneratorAndAssets<T>(*Data.Generator, Min.X + X, Min.Y + Y, Min.Z + Z, 0);
				const T Value = DataHolder.Get(FVoxelDataOctreeUtilities::IndexFromCoordinates(X, Y, Z));
				if (Value != GeneratorValue)
				{
					return false;
				}
			}
		}
	}

	DataHolder.SetIsDirty(false, Data);
	return true;
}

template<typename T>
void FVoxelDataUtilities::SetEntireDataAsDirtyAndCopyFrom(const FVoxelData& SourceData, FVoxelData& DestData)
{
	FVoxelScopedSlowTask SlowTask(1 << (3 * DestData.Depth), VOXEL_LOCTEXT("Copying data to new generator"));
	FVoxelOctreeUtilities::IterateEntireTree(DestData.GetOctree(), [&](FVoxelDataOctreeBase& Tree)
	{
		if (Tree.IsLeaf())
		{
			SlowTask.EnterProgressFrame();

			FVoxelDataOctreeLeaf& Leaf = Tree.AsLeaf();
			const FVoxelDataOctreeBase& SourceBottomNode = FVoxelOctreeUtilities::GetBottomNode(SourceData.GetOctree(), Leaf.Position.X, Leaf.Position.Y, Leaf.Position.Z);
			const FVoxelDataOctreeLeaf* SourceLeaf = SourceBottomNode.IsLeaf() ? &SourceBottomNode.AsLeaf() : nullptr;

			TVoxelDataOctreeLeafData<T>& DataHolder = Leaf.GetData<T>();

			const auto CopyFromGenerator = [&]()
			{
				DataHolder.CreateData(DestData, [&](T* RESTRICT DataPtr)
				{
					TVoxelQueryZone<T> QueryZone(Leaf.GetBounds(), DataPtr);
					SourceBottomNode.GetFromGeneratorAndAssets(*SourceData.Generator, QueryZone, 0); // Note: make sure to use the source generator!
				});
				DataHolder.Compress(DestData); // To save memory
			};

			if (SourceLeaf)
			{
				const TVoxelDataOctreeLeafData<T>& SourceDataHolder = SourceLeaf->GetData<T>();
				if (SourceDataHolder.HasData())
				{
					DataHolder.CreateData(DestData, SourceDataHolder);
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

			DataHolder.SetIsDirty(true, DestData);
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
void FVoxelDataUtilities::CopyDirtyChunksFrom(const FVoxelData& SourceData, FVoxelData& DestData)
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
			DestDataHolder.CreateData(DestData, [&](T* RESTRICT DataPtr)
			{
				FMemory::Memcpy(DataPtr, SrcDataPtr, VOXELS_PER_DATA_CHUNK * sizeof(T));
			});
		}
	});
}

template<typename T>
void FVoxelDataUtilities::OverrideGeneratorValue(FVoxelData& Data, T Value)
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
				DataHolder.SetIsDirty(true, Data);
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

template<typename T>
void FVoxelDataUtilities::ScaleWorldData(const FVoxelData& SourceData, FVoxelData& DestData, const FVoxelVector& Scale)
{
	TArray<FVoxelIntBox> DirtyBounds;
	FVoxelOctreeUtilities::IterateAllLeaves(SourceData.GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
	{
		if (Leaf.GetData<T>().IsDirty())
		{
			const auto ScaledBounds = Leaf.GetBounds().Scale(Scale);
			DirtyBounds.Add(ScaledBounds);
		}
	});

	const FVoxelConstDataAccelerator SourceAccelerator(SourceData, FVoxelIntBox::Infinite);
	FVoxelMutableDataAccelerator DestAccelerator(DestData, FVoxelIntBox::Infinite);

	const auto CopyData = [&](int32 X, int32 Y, int32 Z)
	{
		const auto Position = FVoxelVector(X, Y, Z) / Scale;
		DestAccelerator.Set<T>(X, Y, Z, T(SourceAccelerator.GetFloatValue(Position, 0)));
	};

	FVoxelScopedSlowTask SlowTask(DirtyBounds.Num(), VOXEL_LOCTEXT("Scaling data"));
	for (const FVoxelIntBox& Bounds : DirtyBounds)
	{
		SlowTask.EnterProgressFrame();
		Bounds.Iterate(CopyData);
	}
}