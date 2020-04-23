// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelSave.h"
#include "VoxelSerializationUtilities.h"
#include "VoxelMathUtilities.h"
#include "VoxelCustomVersion.h"
#include "VoxelMessages.h"

#include "Serialization/BufferArchive.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelUncompressedSavesMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelCompressedSavesMemory);

struct FVoxelChunkSaveWithoutFoliage
{
	FIntVector Position;
	int32 ValuesIndex;
	int32 MaterialsIndex;

	FORCEINLINE friend FArchive& operator<<(FArchive& Ar, FVoxelChunkSaveWithoutFoliage& Save)
	{
		Ar << Save.Position;
		Ar << Save.ValuesIndex;
		Ar << Save.MaterialsIndex;

		return Ar;
	}

	FORCEINLINE operator FVoxelUncompressedWorldSaveImpl::FVoxelChunkSave() const
	{
		return { Position, ValuesIndex, MaterialsIndex, -1 };
	}
};

void FVoxelUncompressedWorldSaveImpl::UpdateAllocatedSize() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, AllocatedSize);
	AllocatedSize =
		Chunks.GetAllocatedSize() +
		ValueBuffers.GetAllocatedSize() +
		MaterialBuffers.GetAllocatedSize() +
		FoliageBuffers.GetAllocatedSize() +
		PlaceableItems.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, AllocatedSize);
}

bool FVoxelUncompressedWorldSaveImpl::Serialize(FArchive& Ar)
{
	Ar.UsingCustomVersion(FVoxelCustomVersion::GUID);

	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelCustomVersion::LatestVersion;
		}

		// Serialize version & depth
		{
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
		}

		// Serialize GUID
		if (Version >= FVoxelCustomVersion::ValueConfigFlagAndSaveGUIDs)
		{
			Ar << Guid;
		}
		else
		{
			Guid = FGuid::NewGuid();
		}

		// Serialize UserFlags
		if (Version >= FVoxelCustomVersion::AddUserFlagsToSaves)
		{
			Ar << UserFlags;
		}
		else
		{
			UserFlags = 0;
		}
		
		// Serialize value config
		uint32 ValueConfigFlag = GVoxelValueConfigFlag;
		if (Version >= FVoxelCustomVersion::ValueConfigFlagAndSaveGUIDs)
		{
			Ar << ValueConfigFlag;
		}

		// Serialize material config
		uint32 MaterialConfigFlag = GVoxelMaterialConfigFlag;
		Ar << MaterialConfigFlag;

		// Serialize value buffers
		FVoxelSerializationUtilities::SerializeValues(Ar, ValueBuffers, ValueConfigFlag, Version);

		// Serialize material buffers
		FVoxelSerializationUtilities::SerializeMaterials(Ar, MaterialBuffers, MaterialConfigFlag, Version);

		// Serialize foliage buffers
		if (Version >= FVoxelCustomVersion::FoliagePaint)
		{
			FoliageBuffers.BulkSerialize(Ar);
		}

		// Serialize single values buffers
		if (Version >= FVoxelCustomVersion::SingleValues)
		{
			FVoxelSerializationUtilities::SerializeValues(Ar, SingleValues, ValueConfigFlag, Version);
			FVoxelSerializationUtilities::SerializeMaterials(Ar, SingleMaterials, MaterialConfigFlag, Version);
			SingleFoliage.BulkSerialize(Ar);
		}

		// Serialize chunks indices
		if (Version < FVoxelCustomVersion::FoliagePaint)
		{
			TArray<FVoxelChunkSaveWithoutFoliage> OldChunks;
			if (Version == FVoxelCustomVersion::BeforeCustomVersionWasAdded)
			{
				Ar << OldChunks;
			}
			else
			{
				OldChunks.BulkSerialize(Ar);
			}
			Chunks = TArray<FVoxelChunkSave>(OldChunks);
		}
		else
		{
			Chunks.BulkSerialize(Ar);
		}

		// Serialize placeable items
		if (Version >= FVoxelCustomVersion::PlaceableItemsInSave)
		{
			Ar << PlaceableItems;
		}
		
		if (Ar.IsLoading() && Ar.IsError())
		{
			FVoxelMessages::Error("VoxelSave: Serialization failed, data is corrupted");
			Depth = -1;
			ValueBuffers.Reset();
			MaterialBuffers.Reset();
			Chunks.Reset();
			PlaceableItems.Reset();
		}
		
		UpdateAllocatedSize();
	}

	return true;
}

TArray<uint8> FVoxelUncompressedWorldSaveImpl::GetSerializedData() const
{
	TArray<uint8> Array;
	FMemoryWriter Archive(Array, true);
	const_cast<FVoxelUncompressedWorldSaveImpl*>(this)->Serialize(Archive);
	return MoveTemp(Array);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FVoxelCompressedWorldSaveImpl::~FVoxelCompressedWorldSaveImpl()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, AllocatedSize);
}

bool FVoxelCompressedWorldSaveImpl::Serialize(FArchive& Ar)
{
	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelCustomVersion::LatestVersion;
		}

		Ar << Depth;
		Ar << Version;
		if (Version < FVoxelCustomVersion::ValueConfigFlagAndSaveGUIDs)
		{
			uint32 ConfigFlags;
			Ar << ConfigFlags;
			Guid = FGuid::NewGuid();
		}
		else
		{
			Ar << Guid;
		}
		Ar << CompressedData;

		UpdateAllocatedSize();
	}

	return true;
}

void FVoxelCompressedWorldSaveImpl::UpdateAllocatedSize() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, AllocatedSize);
	AllocatedSize = CompressedData.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelCompressedSavesMemory, AllocatedSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelWorldSaveObject::PostLoad()
{
	Super::PostLoad();
	CopyDepthFromSave();
}

void UVoxelWorldSaveObject::CopyDepthFromSave()
{
	Depth = Save.Const().GetDepth();
}