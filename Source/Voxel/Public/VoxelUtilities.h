// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "VoxelGlobals.h"
#include "IntBox.h"

namespace FVoxelUtilities
{
	template<uint32 ChunkSize>
	inline int GetDepthFromSize(int Size)
	{
		if (Size <= 0)
		{
			return 0;
		}
		else
		{
			return FMath::Clamp(FMath::CeilToInt(FMath::Log2(Size / float(ChunkSize))), 1, MAX_WORLD_DEPTH - 1);
		}
	}
	
	template<uint32 ChunkSize>
	inline int GetSizeFromDepth(int Depth)
	{
		return ChunkSize << Depth;
	}

	template<uint32 ChunkSize>
	inline FIntBox GetBoundsFromDepth(int Depth)
	{
		FIntVector Size = FIntVector((ChunkSize << Depth) / 2);
		return FIntBox(-Size, Size);
	}

	inline int ClampChunkLOD(int LOD)
	{
		return FMath::Clamp(LOD, 0, MAX_WORLD_DEPTH - 1);
	}

	inline int ClampDataLOD(int LOD)
	{
		return FMath::Clamp(LOD, 1, MAX_WORLD_DEPTH - DATA_OCTREE_DEPTH_DIFF - 1);
	}

	inline bool HaveSameSign(const FVoxelValue& A, const FVoxelValue& B)
	{
		return A.IsEmpty() == B.IsEmpty();
	}

	inline void AddMaterialsToArchive(TArray<FVoxelMaterial>& Materials, FArchive& Archive)
	{
		int MaterialsSize = Materials.Num();
		Archive << MaterialsSize;
		for (int I = 0; I < MaterialsSize; I++)
		{
			Archive << Materials[I];
		}
	}

	inline void GetMaterialsFromArchive(TArray<FVoxelMaterial>& Materials, FArchive& Archive, uint32 ConfigFlags)
	{
		if (ConfigFlags == GetVoxelConfigFlags())
		{
			int MaterialsSize;
			Archive << MaterialsSize;
			Materials.SetNumUninitialized(MaterialsSize);
			for (int I = 0; I < MaterialsSize; I++)
			{
				Archive << Materials[I];
			}
		}
		else
		{
			int MaterialsSize;
			Archive << MaterialsSize;
			Materials.SetNumUninitialized(MaterialsSize);
			for (int I = 0; I < MaterialsSize; I++)
			{
				Materials[I] = FVoxelMaterial::SerializeCompat(Archive, ConfigFlags);
			}
		}
	}

	inline void SerializeMaterials(TArray<FVoxelMaterial>& Materials, FArchive& Archive, uint32 ConfigFlags)
	{
		if (Archive.IsLoading())
		{
			GetMaterialsFromArchive(Materials, Archive, ConfigFlags);
		}
		else if (Archive.IsSaving())
		{
			AddMaterialsToArchive(Materials, Archive);
		}
	}

	inline void CompressData(const TArray<uint8>& UncompressedData, TArray<uint8>& CompressedData, ECompressionFlags CompressionFlags = (ECompressionFlags)(COMPRESS_ZLIB | COMPRESS_BiasSpeed))
	{
		int32 UncompressedSize = UncompressedData.Num();
		int32 CompressedSize = FCompression::CompressMemoryBound(CompressionFlags, UncompressedSize);

		CompressedData.SetNumUninitialized(CompressedSize + sizeof(UncompressedSize));

		FMemory::Memcpy(&CompressedData[0], &UncompressedSize, sizeof(UncompressedSize));
		verify(FCompression::CompressMemory(CompressionFlags, CompressedData.GetData() + sizeof(UncompressedSize), CompressedSize, UncompressedData.GetData(), UncompressedData.Num()));
		CompressedData.SetNum(CompressedSize + sizeof(UncompressedSize));

		CompressedData.Add(CompressionFlags);
	}

	inline bool DecompressData(const TArray<uint8>& CompressedData, TArray<uint8>& UncompressedData)
	{
		if (CompressedData.Num() == 0)
		{
			return false;
		}

		ECompressionFlags CompressionFlags = (ECompressionFlags)CompressedData.Last();

		int32 UncompressedSize;
		FMemory::Memcpy(&UncompressedSize, &CompressedData[0], sizeof(UncompressedSize));
		UncompressedData.SetNum(UncompressedSize);
		return FCompression::UncompressMemory(CompressionFlags, UncompressedData.GetData(), UncompressedSize, CompressedData.GetData() + sizeof(UncompressedSize), CompressedData.Num() - 1 - sizeof(UncompressedSize));
	}

	inline void AddNeighborsToArray(const FIntVector& V, TArray<FIntVector>& Array)
	{
		const int32& X = V.X;
		const int32& Y = V.Y;
		const int32& Z = V.Z;

		uint32 Pos = Array.AddUninitialized(6);
		FIntVector* Ptr = Array.GetData() + Pos;

		new (Ptr++) FIntVector(X - 1, Y, Z);
		new (Ptr++) FIntVector(X + 1, Y, Z);

		new (Ptr++) FIntVector(X, Y - 1, Z);
		new (Ptr++) FIntVector(X, Y + 1, Z);

		new (Ptr++) FIntVector(X, Y, Z - 1);
		new (Ptr++) FIntVector(X, Y, Z + 1);

		check(Ptr == Array.GetData() + Array.Num());
	}
};

struct FVoxelTimer
{
public:
	FVoxelTimer()
	{
	}
	
	void Init(float InRateInSeconds)
	{
		RateInSeconds = InRateInSeconds;
		TimeLeft = 0;
	}

	bool Tick(float DeltaTime)
	{
		check(RateInSeconds >= 0);

		TimeLeft -= DeltaTime;
		if (TimeLeft <= 0)
		{
			TimeLeft = RateInSeconds;
			return true;
		}
		return false;
	}

private:
	float RateInSeconds = -1;
	float TimeLeft = 0;
};