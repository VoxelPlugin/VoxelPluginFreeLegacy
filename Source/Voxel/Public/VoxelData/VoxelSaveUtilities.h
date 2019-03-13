// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSave.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VoxelSaveUtilities.generated.h"

class FVoxelSaveBuilder
{
public:
	FVoxelSaveBuilder(int Depth)
		: Depth(Depth)
	{
	}
	void AddChunk(const FIntVector& InPosition, FVoxelValue* InValues, FVoxelMaterial* InMaterials);
	void Save(FVoxelUncompressedWorldSave& OutSave);

private:
	struct FVoxelChunkSaveTmp
	{
		FIntVector Position;
		FVoxelValue* Values;
		FVoxelMaterial* Materials;
	};
	const int Depth;
	uint32 ChunksWithValuesCount = 0;
	uint32 ChunksWithMaterialsCount = 0;
	TArray<FVoxelChunkSaveTmp> TmpChunks;
};

class FVoxelSaveLoader
{
public:
	FVoxelSaveLoader(const FVoxelUncompressedWorldSave& Save)
		: Save(Save)
	{
	}
	
	void CopyChunkToBuffers(int Index, FVoxelValue* DestValues, FVoxelMaterial* DestMaterials, bool& bOutValuesAreSet, bool& bOutMaterialsAreSet) const;

public:
	inline int NumChunks() const
	{
		return Save.Chunks.Num();
	}
	inline FIntVector GetChunkPosition(int Index) const
	{
		return Save.Chunks[Index].Position;
	}

private:
	const FVoxelUncompressedWorldSave& Save;
};

UCLASS()
class VOXEL_API UVoxelSaveUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void CompressVoxelSave(const FVoxelUncompressedWorldSave& UncompressedSave, FVoxelCompressedWorldSave& OutCompressedSave);

	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static bool DecompressVoxelSave(const FVoxelCompressedWorldSave& CompressedSave, FVoxelUncompressedWorldSave& OutUncompressedSave);
};
