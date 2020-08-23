// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelSaveStruct.h"
#include "VoxelCookedData.generated.h"

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Cooked Data Memory"), STAT_VoxelCookedDataMemory, STATGROUP_VoxelMemory, VOXEL_API);

namespace FVoxelCookedDataVersion
{
	enum Type : int32
	{
		BeforeCustomVersionWasAdded,
		SHARED_PlaceableItemsInSave,
		SHARED_AssetItemsImportValueMaterials,
		SHARED_DataAssetScale,
		SHARED_RemoveVoxelGrass,
		SHARED_DataAssetTransform,
		SHARED_RemoveEnableVoxelSpawnedActorsEnableVoxelGrass,
		SHARED_FoliagePaint,
		SHARED_ValueConfigFlagAndSaveGUIDs,
		SHARED_SingleValues,
		SHARED_NoVoxelMaterialInHeightmapAssets,
		SHARED_FixMissingMaterialsInHeightmapAssets,
		SHARED_AddUserFlagsToSaves,
		SHARED_StoreSpawnerMatricesRelativeToComponent,
		SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage,
		
		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};

struct VOXEL_API FVoxelCookedDataImpl
{
	FVoxelCookedDataImpl() = default;
	~FVoxelCookedDataImpl();

	bool operator==(const FVoxelCookedDataImpl& Other) const
	{
		return Guid == Other.Guid;
	}

	bool Serialize(FArchive& Ar);
	void UpdateAllocatedSize() const;

public:
	struct FChunk
	{
		TArray<uint8> Data;

		friend FArchive& operator<<(FArchive& Ar, FChunk& Chunk)
		{
			Chunk.Data.BulkSerialize(Ar);
			return Ar;
		}
	};
	
	void SetNumChunks(int32 Num)
	{
		Chunks.SetNum(Num);
	}
	FChunk& GetChunk(int32 Index)
	{
		return Chunks[Index];
	}
	void RemoveEmptyChunks();

	const TArray<FChunk>& GetChunks() const
	{
		return Chunks;
	}
	bool IsEmpty() const
	{
		return Chunks.Num() == 0;
	}
	
private:
	int32 Version;
	FGuid Guid;

	TArray<FChunk> Chunks;

	mutable int64 AllocatedSize = 0;
};

// Blueprint wrapper that's cheap to copy around
USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelCookedData
#if CPP
	: public TVoxelSaveStruct<FVoxelCookedDataImpl>
#endif
{
	GENERATED_BODY()
};

DEFINE_VOXEL_SAVE_STRUCT(FVoxelCookedData);