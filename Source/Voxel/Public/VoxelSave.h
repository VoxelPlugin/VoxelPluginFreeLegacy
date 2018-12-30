// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "VoxelGlobals.h"
#include "VoxelId.h"
#include "VoxelUtilities.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelLogStatDefinitions.h"
#include "VoxelSave.generated.h"

DECLARE_MEMORY_STAT(TEXT("Voxel Uncompressed Saves Memory"), STAT_VoxelUncompressedSavesMemory, STATGROUP_VoxelMemory);
DECLARE_MEMORY_STAT(TEXT("Voxel Compressed Saves Memory"), STAT_VoxelCompressedSavesMemory, STATGROUP_VoxelMemory);

/**
 *	Uncompressed save of the world
 */
USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelUncompressedWorldSave
{
	GENERATED_BODY()
		
public:
	struct FVoxelChunkSave
	{
		FIntVector Position;
		int ValuesIndex;
		int MaterialsIndex;

		friend inline FArchive& operator<<(FArchive &Ar, FVoxelChunkSave& Save)
		{
			Ar << Save.Position;
			Ar << Save.ValuesIndex;
			Ar << Save.MaterialsIndex;

			return Ar;
		}
	};

public:
	FVoxelUncompressedWorldSave()
	{
		INC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());
	}
	~FVoxelUncompressedWorldSave()
	{
		DEC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());
	}

public:
	inline int GetDepth() const
	{
		return Depth;
	}

	inline int GetAllocatedSize() const
	{
		return Chunks.GetAllocatedSize() + Values.GetAllocatedSize() + Materials.GetAllocatedSize();
	}

public:
	void Init(int InDepth)
	{
		Depth = InDepth;
		ChunksWithValuesCount = 0;
		ChunksWithMaterialsCount = 0;
		TmpChunks.Reset();
	}
	inline void AddChunk(const FIntVector& InPosition, TVoxelBuffer<FVoxelValue>* InValues, TVoxelBuffer<FVoxelMaterial>* InMaterials)
	{
		TmpChunks.Add({ InPosition, InValues, InMaterials });
		if (InValues)
		{
			ChunksWithValuesCount++;
		}
		if (InMaterials)
		{
			ChunksWithMaterialsCount++;
		}
	}
	void Save()
	{
		DEC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());

		check(Depth >= 0);
		Chunks.Empty(TmpChunks.Num());
		Values.Empty(ChunksWithValuesCount * VOXEL_CELL_COUNT);
		Materials.Empty(ChunksWithMaterialsCount * VOXEL_CELL_COUNT);

		for (auto& Chunk : TmpChunks)
		{
			FVoxelChunkSave NewChunk;
			NewChunk.Position = Chunk.Position;
			if (Chunk.Values)
			{
				NewChunk.ValuesIndex = Values.Num();
				Values.AddUninitialized(VOXEL_CELL_COUNT);
				GetValueBuffer(NewChunk.ValuesIndex) = *Chunk.Values;
			}
			else
			{
				NewChunk.ValuesIndex = -1;
			}
			if (Chunk.Materials)
			{
				NewChunk.MaterialsIndex = Materials.Num();
				Materials.AddUninitialized(VOXEL_CELL_COUNT);
				GetMaterialBuffer(NewChunk.MaterialsIndex) = *Chunk.Materials;
			}
			else
			{
				NewChunk.MaterialsIndex = -1;
			}
			Chunks.Add(NewChunk);
		}
		
		TmpChunks.Empty();

		Chunks.Shrink();
		Values.Shrink();
		Materials.Shrink();

		INC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());
	}

public:
	inline int NumChunks() const
	{
		return Chunks.Num();
	}
	inline FIntVector GetChunkPosition(int Index) const
	{
		return Chunks[Index].Position;
	}
	void CopyChunkToBuffers(int Index, TVoxelBuffer<FVoxelValue>& DestValues, TVoxelBuffer<FVoxelMaterial>& DestMaterials, bool& bOutValuesAreSet, bool& bOutMaterialsAreSet) const
	{
		auto& Chunk = Chunks[Index];
		if (Chunk.ValuesIndex >= 0)
		{
			bOutValuesAreSet = true;
			DestValues = GetValueBuffer(Chunk.ValuesIndex);
		}
		else
		{
			bOutValuesAreSet = false;
		}
		if (Chunk.MaterialsIndex >= 0)
		{
			bOutMaterialsAreSet = true;
			DestMaterials = GetMaterialBuffer(Chunk.MaterialsIndex);
		}
		else
		{
			bOutMaterialsAreSet = false;
		}
	}

public:
	bool Serialize(FArchive& Ar)
	{
		if (Ar.IsLoading() || Ar.IsSaving())
		{
			DEC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());

			Ar << Depth;
			uint32 ConfigFlags = GetVoxelConfigFlags();
			Ar << ConfigFlags;
			Ar << Values;
			FVoxelUtilities::SerializeMaterials(Materials, Ar, ConfigFlags);
			Ar << Chunks;

			INC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());
		}

		return true;
	}

	inline bool operator==(const FVoxelUncompressedWorldSave& Other) const
	{
		return Depth == Other.Depth && Chunks == Other.Chunks && Values == Other.Values && Materials == Other.Materials;
	}

private:
	struct FVoxelChunkSaveTmp
	{
		FIntVector Position;
		TVoxelBuffer<FVoxelValue>* Values;
		TVoxelBuffer<FVoxelMaterial>* Materials;
	};
	uint32 ChunksWithValuesCount;
	uint32 ChunksWithMaterialsCount;
	TArray<FVoxelChunkSaveTmp> TmpChunks;

private:
	int Depth = -1;
	TArray<FVoxelValue> Values;
	TArray<FVoxelMaterial> Materials;
	TArray<FVoxelChunkSave> Chunks;

	inline TVoxelBuffer<FVoxelValue>& GetValueBuffer(int Index) const
	{
		return *(TVoxelBuffer<FVoxelValue>*)&Values[Index];
	}
	inline TVoxelBuffer<FVoxelMaterial>& GetMaterialBuffer(int Index) const
	{
		return *(TVoxelBuffer<FVoxelMaterial>*)&Materials[Index];
	}
};	

template <>
struct TTypeTraits<FVoxelUncompressedWorldSave::FVoxelChunkSave> : public TTypeTraitsBase<FVoxelUncompressedWorldSave::FVoxelChunkSave>
{
	enum { IsBytewiseComparable = true };
};

template<>
struct TStructOpsTypeTraits<FVoxelUncompressedWorldSave> : public TStructOpsTypeTraitsBase2<FVoxelUncompressedWorldSave>
{
	enum 
	{
		WithSerializer = true,
		WithIdenticalViaEquality = true
	};
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Compressed save of the world
 */
USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelCompressedWorldSave
{	
	GENERATED_BODY()
		
	FVoxelCompressedWorldSave()
	{
		INC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, GetAllocatedSize());
	}
	~FVoxelCompressedWorldSave()
	{
		DEC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, GetAllocatedSize());
	}

	inline int GetDepth() const
	{
		return Depth;
	}

	bool Serialize(FArchive& Ar)
	{
		if (Ar.IsLoading() || Ar.IsSaving())
		{
			DEC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, GetAllocatedSize());

			Ar << Depth;
			Ar << Version;
			Ar << ConfigFlags;
			Ar << CompressedData;

			INC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, GetAllocatedSize());
		}

		return true;
	}

	inline int GetAllocatedSize() const
	{
		return CompressedData.GetAllocatedSize();
	}

	inline bool operator==(const FVoxelCompressedWorldSave& Other) const
	{
		return Depth == Other.Depth && Version == Other.Version && ConfigFlags == Other.ConfigFlags && CompressedData == Other.CompressedData;
	}
	
private:
	enum EVersion : int
	{
		V0 = 0
	};
	
	int Version;
	int Depth = -1;
	uint32 ConfigFlags;
	TArray<uint8> CompressedData;

	friend class UVoxelSaveUtilities;
};

template<>
struct TStructOpsTypeTraits<FVoxelCompressedWorldSave> : public TStructOpsTypeTraitsBase2<FVoxelCompressedWorldSave>
{
	enum 
	{
		WithSerializer = true,
		WithIdenticalViaEquality = true
	};
};

///////////////////////////////////////////////////////////////////////////////

UCLASS()
class VOXEL_API UVoxelSaveUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void CompressVoxelSave(UPARAM(ref) FVoxelUncompressedWorldSave& UncompressedSave, FVoxelCompressedWorldSave& OutCompressedSave)
	{
		DEC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, OutCompressedSave.GetAllocatedSize());

		FBufferArchive Archive;
		UncompressedSave.Serialize(Archive);
		OutCompressedSave.Depth = UncompressedSave.GetDepth();
		OutCompressedSave.Version = FVoxelCompressedWorldSave::V0;
		OutCompressedSave.ConfigFlags = GetVoxelConfigFlags();
		FVoxelUtilities::CompressData(Archive, OutCompressedSave.CompressedData);
		OutCompressedSave.CompressedData.Shrink();

		INC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, OutCompressedSave.GetAllocatedSize());
	}

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static bool DecompressVoxelSave(const FVoxelCompressedWorldSave& CompressedSave, FVoxelUncompressedWorldSave& OutUncompressedSave)
	{
		if (CompressedSave.CompressedData.Num() == 0)
		{
			return false;
		}
		else
		{
			TArray<uint8> UncompressedData;
			FVoxelUtilities::DecompressData(CompressedSave.CompressedData, UncompressedData);

			FMemoryReader Reader(UncompressedData);
			OutUncompressedSave.Serialize(Reader);
		
			return true;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

UCLASS(Blueprintable, Category = Voxel)
class VOXEL_API UVoxelWorldSaveObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelCompressedWorldSave Save;
};