// Copyright 2019 Phyronnaz

#include "VoxelData/VoxelSave.h"
#include "VoxelData/VoxelSaveUtilities.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "VoxelSerializationUtilities.h"

void FVoxelSaveBuilder::AddChunk(const FIntVector& InPosition, FVoxelValue* InValues, FVoxelMaterial* InMaterials)
{
	if (InValues || InMaterials)
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
}

void FVoxelSaveBuilder::Save(FVoxelUncompressedWorldSave& OutSave)
{
	DEC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, OutSave.GetAllocatedSize());

	check(Depth >= 0);
	OutSave.Depth = Depth;
	OutSave.Chunks.Empty(TmpChunks.Num());
	OutSave.Values.Empty(ChunksWithValuesCount * VOXEL_CELL_COUNT);
	OutSave.Materials.Empty(ChunksWithMaterialsCount * VOXEL_CELL_COUNT);

	for (auto& Chunk : TmpChunks)
	{
		FVoxelUncompressedWorldSave::FVoxelChunkSave NewChunk;
		NewChunk.Position = Chunk.Position;
		if (Chunk.Values)
		{
			NewChunk.ValuesIndex = OutSave.Values.Num();
			OutSave.Values.AddUninitialized(VOXEL_CELL_COUNT);
			FMemory::Memcpy(&OutSave.Values[NewChunk.ValuesIndex], Chunk.Values, sizeof(FVoxelValue) * VOXEL_CELL_COUNT);
		}
		else
		{
			NewChunk.ValuesIndex = -1;
		}
		if (Chunk.Materials)
		{
			NewChunk.MaterialsIndex = OutSave.Materials.Num();
			OutSave.Materials.AddUninitialized(VOXEL_CELL_COUNT);
			FMemory::Memcpy(&OutSave.Materials[NewChunk.MaterialsIndex], Chunk.Materials, sizeof(FVoxelMaterial) *VOXEL_CELL_COUNT);
		}
		else
		{
			NewChunk.MaterialsIndex = -1;
		}
		OutSave.Chunks.Add(NewChunk);
	}

	TmpChunks.Empty();

	OutSave.Chunks.Shrink();
	OutSave.Values.Shrink();
	OutSave.Materials.Shrink();

	INC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, OutSave.GetAllocatedSize());
}

void FVoxelSaveLoader::CopyChunkToBuffers(int Index, FVoxelValue* DestValues, FVoxelMaterial* DestMaterials, bool& bOutValuesAreSet, bool& bOutMaterialsAreSet) const
{
	auto& Chunk = Save.Chunks[Index];
	if (Chunk.ValuesIndex >= 0)
	{
		bOutValuesAreSet = true;
		FMemory::Memcpy(DestValues, &Save.Values[Chunk.ValuesIndex], sizeof(FVoxelValue) *VOXEL_CELL_COUNT);
	}
	else
	{
		bOutValuesAreSet = false;
	}
	if (Chunk.MaterialsIndex >= 0)
	{
		bOutMaterialsAreSet = true;
		FMemory::Memcpy(DestMaterials, &Save.Materials[Chunk.MaterialsIndex], sizeof(FVoxelMaterial) *VOXEL_CELL_COUNT);
	}
	else
	{
		bOutMaterialsAreSet = false;
	}
}

void UVoxelSaveUtilities::CompressVoxelSave(const FVoxelUncompressedWorldSave& UncompressedSave, FVoxelCompressedWorldSave& OutCompressedSave)
{
	DEC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, OutCompressedSave.GetAllocatedSize());

	OutCompressedSave.Depth = UncompressedSave.GetDepth();
	OutCompressedSave.Version = FVoxelCompressedWorldSave::V0;
	OutCompressedSave.ConfigFlags = GetVoxelConfigFlags();
	FVoxelSerializationUtilities::CompressData(UncompressedSave.GetSerializedData(), OutCompressedSave.CompressedData);
	OutCompressedSave.CompressedData.Shrink();

	INC_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, OutCompressedSave.GetAllocatedSize());
}

bool UVoxelSaveUtilities::DecompressVoxelSave(const FVoxelCompressedWorldSave& CompressedSave, FVoxelUncompressedWorldSave& OutUncompressedSave)
{
	if (CompressedSave.CompressedData.Num() == 0)
	{
		return false;
	}
	else
	{
		TArray<uint8> UncompressedData;
		FVoxelSerializationUtilities::DecompressData(CompressedSave.CompressedData, UncompressedData);

		FMemoryReader Reader(UncompressedData);
		OutUncompressedSave.Serialize(Reader);

		return true;
	}
}

bool FVoxelUncompressedWorldSave::Serialize(FArchive& Ar)
{
	if (Ar.IsLoading() || Ar.IsSaving())
	{
		DEC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());

		Ar << Depth;
		uint32 ConfigFlags = GetVoxelConfigFlags();
		Ar << ConfigFlags;
		Ar << Values;
		FVoxelSerializationUtilities::SerializeMaterials(Materials, Ar, ConfigFlags);
		Ar << Chunks;

		INC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());
	}

	return true;
}

TArray<uint8> FVoxelUncompressedWorldSave::GetSerializedData() const
{
	FBufferArchive Archive;
	const_cast<FVoxelUncompressedWorldSave*>(this)->Serialize(Archive);
	return Archive;
}

bool FVoxelCompressedWorldSave::Serialize(FArchive& Ar)
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

void UVoxelWorldSaveObject::PostLoad()
{
	Super::PostLoad();
	Depth = Save.GetDepth() - DATA_OCTREE_DEPTH_DIFF;
}

#if WITH_EDITOR
void UVoxelWorldSaveObject::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Depth = Save.GetDepth() - DATA_OCTREE_DEPTH_DIFF;
}
#endif
