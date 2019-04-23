// Copyright 2019 Phyronnaz

#include "VoxelData/VoxelSave.h"
#include "VoxelSerializationUtilities.h"
#include "VoxelMathUtilities.h"
#include "VoxelCustomVersion.h"
#include "VoxelBlueprintErrors.h"

#include "Serialization/BufferArchive.h"

bool FVoxelUncompressedWorldSave::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FVoxelCustomVersion::GUID);

	if (Ar.IsLoading() || Ar.IsSaving())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelCustomVersion::LatestVersion;
		}

		DEC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());

		int32 Dummy = 42;
		Ar << Dummy;
		if (Dummy == 42) // Trick to know the version, as Depth is always smaller than 42
		{
			Ar << Version;
			Ar << Depth;
		}
		else
		{
			Version = FVoxelCustomVersion::BeforeCustomVersionWasAdded;
			Depth = Dummy;
		}
		uint32 ConfigFlags = GetVoxelMaterialConfigFlag();
		Ar << ConfigFlags;
		if (Version == FVoxelCustomVersion::BeforeCustomVersionWasAdded)
		{
			Ar << Values;
		}
		else
		{
			Values.BulkSerialize(Ar);
		}
		FVoxelSerializationUtilities::SerializeMaterials(Ar, Materials, ConfigFlags, Version);
		if (Version == FVoxelCustomVersion::BeforeCustomVersionWasAdded)
		{
			Ar << Chunks;
		}
		else
		{
			Chunks.BulkSerialize(Ar);
		}
		if (Version >= FVoxelCustomVersion::PlaceableItemsInSave)
		{
			Ar << PlaceableItems;
		}
		
		if (Ar.IsLoading() && Ar.IsError())
		{
			FVoxelBPErrors::Error(NSLOCTEXT("Voxel", "VoxelSaveSerializationFailed", "VoxelSave: Serialization failed, data is corrupted"));
			Depth = -1;
			Values.Reset();
			Materials.Reset();
			Chunks.Reset();
			PlaceableItems.Reset();
		}

		INC_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, GetAllocatedSize());
	}

	return true;
}

TArray<uint8> FVoxelUncompressedWorldSave::GetSerializedData() const
{
	FBufferArchive Archive;
	const_cast<FVoxelUncompressedWorldSave*>(this)->Serialize(Archive);
	return MoveTemp(Archive);
}

bool FVoxelCompressedWorldSave::Serialize(FArchive& Ar)
{
	if (Ar.IsLoading() || Ar.IsSaving())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelCustomVersion::LatestVersion;
		}

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
	Depth = FVoxelUtilities::GetChunkDepthFromDataDepth(Save.GetDepth());
}

#if WITH_EDITOR
void UVoxelWorldSaveObject::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	Depth = FVoxelUtilities::GetChunkDepthFromDataDepth(Save.GetDepth());
}
#endif
