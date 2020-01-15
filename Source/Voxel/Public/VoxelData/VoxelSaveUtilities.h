// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSave.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelSaveUtilities.generated.h"

struct FVoxelFoliage;
class FVoxelPlaceableItem;
class AVoxelWorld;
template<typename T>
class TVoxelDataOctreeLeafData;

class FVoxelSaveBuilder
{
public:
	explicit FVoxelSaveBuilder(int32 Depth)
		: Depth(Depth)
	{
	}
	void AddChunk(
		const FIntVector& InPosition,
		const TVoxelDataOctreeLeafData<FVoxelValue>& InValues,
		const TVoxelDataOctreeLeafData<FVoxelMaterial>& InMaterials,
		const TVoxelDataOctreeLeafData<FVoxelFoliage>& InFoliage);
	void AddPlaceableItem(const TVoxelSharedPtr<FVoxelPlaceableItem>& PlaceableItem);
	void Save(FVoxelUncompressedWorldSave& OutSave);

private:
	struct FChunkToSave
	{
		template<typename T>
		struct TData
		{
			T* RESTRICT DataPtr = nullptr;
			bool bIsSingleValue = false;
			T SingleValue;
		};
		
		FIntVector Position;
		TData<FVoxelValue> Values;
		TData<FVoxelMaterial> Materials;
		TData<FVoxelFoliage> Foliage;
	};
	const int32 Depth;
	TArray<FChunkToSave> ChunksToSave;
	TArray<TVoxelSharedPtr<FVoxelPlaceableItem>> PlaceableItems;
};

class FVoxelSaveLoader
{
public:
	explicit FVoxelSaveLoader(const FVoxelUncompressedWorldSave& Save)
		: Save(Save)
	{
	}

	void ExtractChunk(
		int32 ChunkIndex,
		TVoxelDataOctreeLeafData<FVoxelValue>& OutValues,
		TVoxelDataOctreeLeafData<FVoxelMaterial>& OutMaterials,
		TVoxelDataOctreeLeafData<FVoxelFoliage>& OutFoliage) const;
	TArray<TVoxelSharedPtr<FVoxelPlaceableItem>> GetPlaceableItems(const AVoxelWorld * VoxelWorld);

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
	const FVoxelUncompressedWorldSave& Save;
	bool bError = false;
};

UCLASS()
class VOXEL_API UVoxelSaveUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Save")
	static void CompressVoxelSave(const FVoxelUncompressedWorldSave& UncompressedSave, FVoxelCompressedWorldSave& OutCompressedSave);

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data|Save")
	static bool DecompressVoxelSave(const FVoxelCompressedWorldSave& CompressedSave, FVoxelUncompressedWorldSave& OutUncompressedSave);
};