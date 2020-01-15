// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelData/VoxelDataOctreeLeafData.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelMessages.h"
#include "VoxelSerializationUtilities.h"
#include "VoxelCustomVersion.h"

#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

constexpr int32 GSingleValueIndexFlag = 1 << 30;

void FVoxelSaveBuilder::AddChunk(
	const FIntVector& InPosition,
	const TVoxelDataOctreeLeafData<FVoxelValue>& InValues,
	const TVoxelDataOctreeLeafData<FVoxelMaterial>& InMaterials,
	const TVoxelDataOctreeLeafData<FVoxelFoliage>& InFoliage)
{
	if (InValues.IsDirty() || InMaterials.IsDirty() || InFoliage.IsDirty())
	{
		const auto GetData = [](auto& InData)
		{
			FChunkToSave::TData<decltype(InData.GetSingleValue())> Result;
			
			if (InData.IsSingleValue())
			{
				Result.bIsSingleValue = true;
				Result.SingleValue = InData.GetSingleValue();
			}
			else
			{
				Result.DataPtr = InData.GetDataPtr();
			}
			
			return Result;
		};
		ChunksToSave.Add({ InPosition, GetData(InValues), GetData(InMaterials), GetData(InFoliage) });
	}
}

void FVoxelSaveBuilder::Save(FVoxelUncompressedWorldSave& OutSave)
{
	VOXEL_FUNCTION_COUNTER();
	
	DEC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, OutSave.GetAllocatedSize());

	check(Depth >= 0);
	OutSave.Guid = FGuid::NewGuid();
	OutSave.Depth = Depth;
	OutSave.Chunks.Empty(ChunksToSave.Num());

	{
		uint32 ChunksWithValueBuffer = 0;
		uint32 ChunksWithMaterialBuffer = 0;
		uint32 ChunksWithFoliageBuffer = 0;
		
		uint32 ChunksWithSingleValue = 0;
		uint32 ChunksWithSingleMaterial = 0;
		uint32 ChunksWithSingleFoliage = 0;
		
		for (auto& Chunk : ChunksToSave)
		{
			ChunksWithValueBuffer += Chunk.Values.DataPtr != nullptr;
			ChunksWithMaterialBuffer += Chunk.Materials.DataPtr != nullptr;
			ChunksWithFoliageBuffer += Chunk.Foliage.DataPtr != nullptr;
			
			ChunksWithSingleValue += Chunk.Values.bIsSingleValue;
			ChunksWithSingleMaterial += Chunk.Materials.bIsSingleValue;
			ChunksWithSingleFoliage += Chunk.Foliage.bIsSingleValue;
		}
		
		OutSave.ValueBuffers.Empty(ChunksWithValueBuffer * VOXELS_PER_DATA_CHUNK);
		OutSave.MaterialBuffers.Empty(ChunksWithMaterialBuffer * VOXELS_PER_DATA_CHUNK);
		OutSave.FoliageBuffers.Empty(ChunksWithFoliageBuffer * VOXELS_PER_DATA_CHUNK);
		
		OutSave.SingleValues.Empty(ChunksWithSingleValue);
		OutSave.SingleMaterials.Empty(ChunksWithSingleMaterial);
		OutSave.SingleFoliage.Empty(ChunksWithSingleFoliage);
	}

	for (auto& Chunk : ChunksToSave)
	{
		FVoxelUncompressedWorldSave::FVoxelChunkSave NewChunk;
		NewChunk.Position = Chunk.Position;
		if (Chunk.Values.DataPtr)
		{
			NewChunk.ValuesIndex = OutSave.ValueBuffers.Num();
			check(OutSave.ValueBuffers.GetSlack() >= VOXELS_PER_DATA_CHUNK);
			OutSave.ValueBuffers.AddUninitialized(VOXELS_PER_DATA_CHUNK);
			FMemory::Memcpy(&OutSave.ValueBuffers[NewChunk.ValuesIndex], Chunk.Values.DataPtr, sizeof(FVoxelValue) * VOXELS_PER_DATA_CHUNK);
		}
		else if (Chunk.Values.bIsSingleValue)
		{
			NewChunk.ValuesIndex = OutSave.SingleValues.Num() | GSingleValueIndexFlag;
			check(OutSave.SingleValues.GetSlack() > 0);
			OutSave.SingleValues.Add(Chunk.Values.SingleValue);
		}
		else
		{
			NewChunk.ValuesIndex = -1;
		}
		if (Chunk.Materials.DataPtr)
		{
			NewChunk.MaterialsIndex = OutSave.MaterialBuffers.Num();
			check(OutSave.MaterialBuffers.GetSlack() >= VOXELS_PER_DATA_CHUNK);
			OutSave.MaterialBuffers.AddUninitialized(VOXELS_PER_DATA_CHUNK);
			FMemory::Memcpy(&OutSave.MaterialBuffers[NewChunk.MaterialsIndex], Chunk.Materials.DataPtr, sizeof(FVoxelMaterial) * VOXELS_PER_DATA_CHUNK);
		}
		else if (Chunk.Materials.bIsSingleValue)
		{
			NewChunk.MaterialsIndex = OutSave.SingleMaterials.Num() | GSingleValueIndexFlag;
			check(OutSave.SingleMaterials.GetSlack() > 0);
			OutSave.SingleMaterials.Add(Chunk.Materials.SingleValue);
		}
		else
		{
			NewChunk.MaterialsIndex = -1;
		}
		if (Chunk.Foliage.DataPtr)
		{
			NewChunk.FoliageIndex = OutSave.FoliageBuffers.Num();
			check(OutSave.FoliageBuffers.GetSlack() >= VOXELS_PER_DATA_CHUNK);
			OutSave.FoliageBuffers.AddUninitialized(VOXELS_PER_DATA_CHUNK);
			FMemory::Memcpy(&OutSave.FoliageBuffers[NewChunk.FoliageIndex], Chunk.Foliage.DataPtr, sizeof(FVoxelFoliage) * VOXELS_PER_DATA_CHUNK);
		}
		else if (Chunk.Foliage.bIsSingleValue)
		{
			NewChunk.FoliageIndex = OutSave.SingleFoliage.Num() | GSingleValueIndexFlag;
			check(OutSave.SingleFoliage.GetSlack() > 0);
			OutSave.SingleFoliage.Add(Chunk.Foliage.SingleValue);
		}
		else
		{
			NewChunk.FoliageIndex = -1;
		}
		OutSave.Chunks.Add(NewChunk);
	}

	ChunksToSave.Empty();
	
	FMemoryWriter Writer(OutSave.PlaceableItems, true);
	int32 Num = PlaceableItems.Num();
	Writer << Num;
	for (auto& Item : PlaceableItems)
	{
		SerializeVoxelItem(Writer, nullptr, Item);
	}
	OutSave.PlaceableItems.Shrink();

	INC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, OutSave.GetAllocatedSize());
}

void FVoxelSaveBuilder::AddPlaceableItem(const TVoxelSharedPtr<FVoxelPlaceableItem>& PlaceableItem)
{
	PlaceableItems.Add(PlaceableItem);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSaveLoader::ExtractChunk(
	int32 ChunkIndex, 
	TVoxelDataOctreeLeafData<FVoxelValue>& OutValues, 
	TVoxelDataOctreeLeafData<FVoxelMaterial>& OutMaterials,
	TVoxelDataOctreeLeafData<FVoxelFoliage>& OutFoliage) const
{
	OutValues.ClearData();
	OutMaterials.ClearData();
	OutFoliage.ClearData();
	
	auto& Chunk = Save.Chunks[ChunkIndex];
	if (Chunk.ValuesIndex >= 0)
	{
		if (Chunk.ValuesIndex & GSingleValueIndexFlag)
		{
			OutValues.SetSingleValue(Save.SingleValues[Chunk.ValuesIndex & (~GSingleValueIndexFlag)]);
		}
		else
		{
			OutValues.CreateDataPtr();
			check(Save.ValueBuffers.Num() >= Chunk.ValuesIndex + VOXELS_PER_DATA_CHUNK);
			FMemory::Memcpy(OutValues.GetDataPtr(), &Save.ValueBuffers[Chunk.ValuesIndex], sizeof(FVoxelValue) * VOXELS_PER_DATA_CHUNK);
		}
		OutValues.SetDirty();
	}
	if (Chunk.MaterialsIndex >= 0)
	{
		if (Chunk.MaterialsIndex & GSingleValueIndexFlag)
		{
			OutMaterials.SetSingleValue(Save.SingleMaterials[Chunk.MaterialsIndex & (~GSingleValueIndexFlag)]);
		}
		else
		{
			OutMaterials.CreateDataPtr();
			check(Save.MaterialBuffers.Num() >= Chunk.MaterialsIndex + VOXELS_PER_DATA_CHUNK);
			FMemory::Memcpy(OutMaterials.GetDataPtr(), &Save.MaterialBuffers[Chunk.MaterialsIndex], sizeof(FVoxelMaterial) * VOXELS_PER_DATA_CHUNK);
		}
		OutMaterials.SetDirty();
	}
	if (Chunk.FoliageIndex >= 0)
	{
		if (Chunk.FoliageIndex & GSingleValueIndexFlag)
		{
			OutFoliage.SetSingleValue(Save.SingleFoliage[Chunk.FoliageIndex & (~GSingleValueIndexFlag)]);
		}
		else
		{
			OutFoliage.CreateDataPtr();
			check(Save.FoliageBuffers.Num() >= Chunk.FoliageIndex + VOXELS_PER_DATA_CHUNK);
			FMemory::Memcpy(OutFoliage.GetDataPtr(), &Save.FoliageBuffers[Chunk.FoliageIndex], sizeof(FVoxelFoliage) * VOXELS_PER_DATA_CHUNK);
		}
		OutFoliage.SetDirty();
	}
}

TArray<TVoxelSharedPtr<FVoxelPlaceableItem>> FVoxelSaveLoader::GetPlaceableItems(const AVoxelWorld* VoxelWorld)
{
	VOXEL_FUNCTION_COUNTER();
	
	FMemoryReader Reader(Save.PlaceableItems);
	Reader.SetCustomVersion(FVoxelCustomVersion::GUID, Save.Version, "VoxelCustomVersion");
	TArray<TVoxelSharedPtr<FVoxelPlaceableItem>> PlaceableItems;
	int32 Num;
	Reader << Num;
	for (int32 Index = 0; Index < Num; Index++)
	{
		PlaceableItems.Emplace();
		SerializeVoxelItem(Reader, VoxelWorld, PlaceableItems.Last());
	}
	bError |= Reader.GetError();
	return PlaceableItems;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSaveUtilities::CompressVoxelSave(const FVoxelUncompressedWorldSave& UncompressedSave, FVoxelCompressedWorldSave& OutCompressedSave)
{
	VOXEL_FUNCTION_COUNTER();
	
	DEC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, OutCompressedSave.GetAllocatedSize());

	OutCompressedSave.Depth = UncompressedSave.GetDepth();
	OutCompressedSave.Guid = UncompressedSave.GetGuid();
	FVoxelSerializationUtilities::CompressData(UncompressedSave.GetSerializedData(), OutCompressedSave.CompressedData);
	OutCompressedSave.CompressedData.Shrink();

	INC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, OutCompressedSave.GetAllocatedSize());
}

bool UVoxelSaveUtilities::DecompressVoxelSave(const FVoxelCompressedWorldSave& CompressedSave, FVoxelUncompressedWorldSave& OutUncompressedSave)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (CompressedSave.CompressedData.Num() == 0)
	{
		return false;
	}
	else
	{
		TArray<uint8> UncompressedData;
		if (!FVoxelSerializationUtilities::DecompressData(CompressedSave.CompressedData, UncompressedData))
		{
			FVoxelMessages::Error(NSLOCTEXT("Voxel", "DecompressVoxelSaveFailed", "DecompressVoxelSave failed: Corrupted data"));
			return false;
		}

		FMemoryReader Reader(UncompressedData);
		OutUncompressedSave.Serialize(Reader);

		return true;
	}
}