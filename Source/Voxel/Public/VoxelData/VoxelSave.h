// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
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
	bool Serialize(FArchive& Ar);
	TArray<uint8> GetSerializedData() const;

	inline bool operator==(const FVoxelUncompressedWorldSave& Other) const
	{
		return Depth == Other.Depth && Chunks == Other.Chunks && Values == Other.Values && Materials == Other.Materials;
	}

private:
	int Depth = -1;
	TArray<FVoxelValue> Values;
	TArray<FVoxelMaterial> Materials;
	TArray<FVoxelChunkSave> Chunks;

	friend class FVoxelSaveBuilder;
	friend class FVoxelSaveLoader;
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

	bool Serialize(FArchive& Ar);

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

UCLASS(Blueprintable, Category = Voxel)
class VOXEL_API UVoxelWorldSaveObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVoxelCompressedWorldSave Save;

	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	int Depth = 0;

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};