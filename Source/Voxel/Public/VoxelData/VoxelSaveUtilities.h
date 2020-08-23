// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSave.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelSaveUtilities.generated.h"

struct FVoxelAssetItem;
struct FVoxelPlaceableItemLoadInfo;

class AVoxelWorld;
class IVoxelDataOctreeMemory;
template<typename T>
class TVoxelDataOctreeLeafData;

class FVoxelSaveBuilder
{
public:
	explicit FVoxelSaveBuilder(int32 Depth);

	void AddChunk(
		const FIntVector& Position,
		const TVoxelDataOctreeLeafData<FVoxelValue>& Values,
		const TVoxelDataOctreeLeafData<FVoxelMaterial>& Materials)
	{
		ChunksToSave.Add({ Position, &Values, &Materials });
	}

	void AddAssetItem(const FVoxelAssetItem& AssetItem);

	void Save(FVoxelUncompressedWorldSaveImpl& OutSave, TArray<FVoxelObjectArchiveEntry>& OutObjects);

private:
	struct FChunkToSave
	{
		FIntVector Position;
		const TVoxelDataOctreeLeafData<FVoxelValue>* Values = nullptr;
		const TVoxelDataOctreeLeafData<FVoxelMaterial>* Materials = nullptr;
	};
	const int32 Depth;
	TArray<FChunkToSave> ChunksToSave;
	TArray<FVoxelAssetItem> AssetItems;
};

class FVoxelSaveLoader
{
public:
	explicit FVoxelSaveLoader(const FVoxelUncompressedWorldSaveImpl& Save)
		: Save(Save)
	{
	}

	void ExtractChunk(
		int32 ChunkIndex,
		const IVoxelDataOctreeMemory& Memory,
		TVoxelDataOctreeLeafData<FVoxelValue>& OutValues,
		TVoxelDataOctreeLeafData<FVoxelMaterial>& OutMaterials) const;
	
	void GetPlaceableItems(const FVoxelPlaceableItemLoadInfo& LoadInfo, TArray<FVoxelAssetItem>& OutAssetItems);

public:
	int32 NumChunks() const
	{
		return Save.Chunks.Num();
	}
	FIntVector GetChunkPosition(int32 ChunkIndex) const
	{
		return Save.Chunks[ChunkIndex].Position;
	}
	bool GetError() const
	{
		return bError;
	}

private:
	const FVoxelUncompressedWorldSaveImpl& Save;
	bool bError = false;
};

UCLASS()
class VOXEL_API UVoxelSaveUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Save")
	static void CompressVoxelSave(const FVoxelUncompressedWorldSave& UncompressedSave, FVoxelCompressedWorldSave& OutCompressedSave);
	static void CompressVoxelSave(const FVoxelUncompressedWorldSaveImpl& UncompressedSave, FVoxelCompressedWorldSaveImpl& OutCompressedSave);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Save")
	static bool DecompressVoxelSave(const FVoxelCompressedWorldSave& CompressedSave, FVoxelUncompressedWorldSave& OutUncompressedSave);
	static bool DecompressVoxelSave(const FVoxelCompressedWorldSaveImpl& CompressedSave, FVoxelUncompressedWorldSaveImpl& OutUncompressedSave);
};