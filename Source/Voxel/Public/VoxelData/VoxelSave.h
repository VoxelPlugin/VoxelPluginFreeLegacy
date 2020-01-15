// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelFoliage.h"
#include "VoxelSave.generated.h"

DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel Uncompressed Saves Memory"), STAT_VoxelUncompressedSavesMemory, STATGROUP_VoxelMemory, VOXEL_API);
DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel Compressed Saves Memory"), STAT_VoxelCompressedSavesMemory, STATGROUP_VoxelMemory, VOXEL_API);

/**
 *	Uncompressed save of the world
 */
USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelUncompressedWorldSave
{
	GENERATED_BODY()

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
	int32 GetDepth() const
	{
		return Depth;
	}
	FGuid GetGuid() const
	{
		return Guid;
	}
	int32 GetAllocatedSize() const
	{
		return 
			Chunks.GetAllocatedSize() +
			ValueBuffers.GetAllocatedSize() + 
			MaterialBuffers.GetAllocatedSize() +
			FoliageBuffers.GetAllocatedSize() +
			PlaceableItems.GetAllocatedSize();
	}

public:
	bool Serialize(FArchive& Ar);
	TArray<uint8> GetSerializedData() const;

	bool operator==(const FVoxelUncompressedWorldSave& Other) const
	{
		return Guid == Other.Guid;
	}

private:
	struct FVoxelChunkSave
	{
		FIntVector Position;
		// GSingleValueIndexFlag bit is set if it's an index to the single value buffers
		int32 ValuesIndex = -1;
		int32 MaterialsIndex = -1;
		int32 FoliageIndex = -1;

		friend FArchive& operator<<(FArchive& Ar, FVoxelChunkSave& Save)
		{
			Ar << Save.Position;
			Ar << Save.ValuesIndex;
			Ar << Save.MaterialsIndex;
			Ar << Save.FoliageIndex;
			return Ar;
		}
	};
	
	int32 Version = -1;
	FGuid Guid;
	int32 Depth = -1;
	
	TArray<FVoxelValue> ValueBuffers;
	TArray<FVoxelMaterial> MaterialBuffers;
	TArray<FVoxelFoliage> FoliageBuffers;
	
	TArray<FVoxelValue> SingleValues;
	TArray<FVoxelMaterial> SingleMaterials;
	TArray<FVoxelFoliage> SingleFoliage;
	
	TArray<FVoxelChunkSave> Chunks;
	TArray<uint8> PlaceableItems;

	friend class FVoxelSaveBuilder;
	friend class FVoxelSaveLoader;
	friend struct FVoxelChunkSaveWithoutFoliage;
};

inline FArchive& operator<<(FArchive &Ar, FVoxelUncompressedWorldSave& Save)
{
	Save.Serialize(Ar);
	return Ar;
}

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

	inline int32 GetDepth() const
	{
		return Depth;
	}

	bool Serialize(FArchive& Ar);

	int32 GetAllocatedSize() const
	{
		return CompressedData.GetAllocatedSize();
	}

	bool operator==(const FVoxelCompressedWorldSave& Other) const
	{
		return Guid == Other.Guid;
	}
	
private:
	int32 Version;
	FGuid Guid;
	int32 Depth = -1;
	TArray<uint8> CompressedData;

	friend class UVoxelSaveUtilities;
};

inline FArchive& operator<<(FArchive &Ar, FVoxelCompressedWorldSave& Save)
{
	Save.Serialize(Ar);
	return Ar;
}

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

	// Depth of the world
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	int32 Depth = 0;

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};