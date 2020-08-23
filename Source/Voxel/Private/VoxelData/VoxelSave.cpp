// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelSave.h"
#include "VoxelUtilities/VoxelSerializationUtilities.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelMessages.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelUncompressedSavesMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelCompressedSavesMemory);

struct FVoxelChunkSave32Bits
{
	FIntVector Position;
	int32 ValuesIndex = -1;
	int32 MaterialsIndex = -1;
	int32 FoliageIndex = -1;

	friend FArchive& operator<<(FArchive& Ar, FVoxelChunkSave32Bits& Save)
	{
		Ar << Save.Position;
		Ar << Save.ValuesIndex;
		Ar << Save.MaterialsIndex;
		Ar << Save.FoliageIndex;
		return Ar;
	}
};

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

	FORCEINLINE operator FVoxelChunkSave32Bits() const
	{
		return { Position, ValuesIndex, MaterialsIndex, -1 };
	}
};

struct FVoxelFoliage
{
	uint8 R;
	uint8 G;
	uint8 B;
	uint8 A;
	
	inline friend FArchive& operator<<(FArchive& Ar, FVoxelFoliage& Foliage)
	{
		Ar << Foliage.R;
		Ar << Foliage.G;
		Ar << Foliage.B;
		Ar << Foliage.A;
		return Ar;
	}
};

void FVoxelUncompressedWorldSaveImpl::UpdateAllocatedSize() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, AllocatedSize);
	AllocatedSize =
		Chunks.GetAllocatedSize() +
		ValueBuffers.GetAllocatedSize() +
		MaterialBuffers.GetAllocatedSize() +
		PlaceableItems.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelUncompressedSavesMemory, AllocatedSize);
}

bool FVoxelUncompressedWorldSaveImpl::Serialize(FArchive& Ar)
{
	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelSaveVersion::LatestVersion;
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
				Version = FVoxelSaveVersion::BeforeCustomVersionWasAdded;
				Depth = Dummy;
			}
		}
		
		const auto SerializationVersion =
			Version >= FVoxelSaveVersion::ValueConfigFlagAndSaveGUIDs
			? FVoxelSerializationVersion::ValueConfigFlagAndSaveGUIDs
			: Version >= FVoxelSaveVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass
			? FVoxelSerializationVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass
			: FVoxelSerializationVersion::BeforeCustomVersionWasAdded;

		static_assert(FVoxelSerializationVersion::LatestVersion == FVoxelSerializationVersion::SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage, "Need to add a new FVoxelSaveVersion");

		// Serialize GUID
		if (Version >= FVoxelSaveVersion::ValueConfigFlagAndSaveGUIDs)
		{
			Ar << Guid;
		}
		else
		{
			Guid = FGuid::NewGuid();
		}

		// Serialize UserFlags
		if (Version >= FVoxelSaveVersion::AddUserFlagsToSaves)
		{
			Ar << UserFlags;
		}
		else
		{
			UserFlags = 0;
		}
		
		// Serialize value config
		uint32 ValueConfigFlag = GVoxelValueConfigFlag;
		if (Version >= FVoxelSaveVersion::ValueConfigFlagAndSaveGUIDs)
		{
			Ar << ValueConfigFlag;
		}

		// Serialize material config
		uint32 MaterialConfigFlag = GVoxelMaterialConfigFlag;
		Ar << MaterialConfigFlag;

		// Serialize buffers
		if (Version >= FVoxelSaveVersion::StoreMaterialChannelsIndividuallyAndRemoveFoliage)
		{
			// Serialize value buffers
			FVoxelSerializationUtilities::SerializeValues(Ar, ValueBuffers, ValueConfigFlag, SerializationVersion);
			FVoxelSerializationUtilities::SerializeValues(Ar, SingleValues, ValueConfigFlag, SerializationVersion);

			// Serialize material buffers
			FVoxelSerializationUtilities::SerializeMaterials(Ar, MaterialsIndices, MaterialConfigFlag);
			MaterialBuffers.BulkSerialize(Ar);
			SingleMaterials.BulkSerialize(Ar);

			// Serialize chunks indices
			// Note: make sure to not use BulkSerialize as data isn't aligned
			Ar << Chunks;
		}
		else
		{
			TNoGrowArray<FVoxelMaterial> OldMaterialBuffers;
			TNoGrowArray<FVoxelMaterial> OldSingleMaterials;
			
			// Serialize value buffers
			FVoxelSerializationUtilities::SerializeValues(Ar, ValueBuffers, ValueConfigFlag, SerializationVersion);

			// Serialize material buffers
			FVoxelSerializationUtilities::SerializeMaterials(Ar, OldMaterialBuffers, MaterialConfigFlag, SerializationVersion);

			// Serialize foliage buffers
			if (Version >= FVoxelSaveVersion::FoliagePaint)
			{
				TArray<FVoxelFoliage> FoliageBuffers;
				FoliageBuffers.BulkSerialize(Ar);
			}

			// Serialize single values buffers
			if (Version >= FVoxelSaveVersion::SingleValues)
			{
				FVoxelSerializationUtilities::SerializeValues(Ar, SingleValues, ValueConfigFlag, SerializationVersion);
				FVoxelSerializationUtilities::SerializeMaterials(Ar, OldSingleMaterials, MaterialConfigFlag, SerializationVersion);

				TArray<FVoxelFoliage> SingleFoliage;
				SingleFoliage.BulkSerialize(Ar);
			}

			// Serialize chunks indices
			struct FVoxelChunkSaveWithSingleMaterial
			{
				FIntVector Position;

				int32 ValuesIndex = -1;
				int32 MaterialsIndex = -1;

				bool bSingleValue = false;
				// Makes life easier when loading legacy files
				bool bSingleMaterial_Unused = false;
			};
			TNoGrowArray<FVoxelChunkSaveWithSingleMaterial> NewChunks;
			{
				TArray<FVoxelChunkSave32Bits> OldChunks;
				if (Version < FVoxelSaveVersion::FoliagePaint)
				{
					TArray<FVoxelChunkSaveWithoutFoliage> ChunksWithoutFoliage;
					if (Version == FVoxelSaveVersion::BeforeCustomVersionWasAdded)
					{
						Ar << ChunksWithoutFoliage;
					}
					else
					{
						ChunksWithoutFoliage.BulkSerialize(Ar);
					}
					OldChunks = TArray<FVoxelChunkSave32Bits>(ChunksWithoutFoliage);
				}
				else
				{
					OldChunks.BulkSerialize(Ar);
				}

				NewChunks.Empty(OldChunks.Num());
				for (auto& OldChunk : OldChunks)
				{
					constexpr int32 SingleValueIndexFlag = 1 << 30;

					FVoxelChunkSaveWithSingleMaterial& NewChunk = NewChunks.Emplace_GetRef();

					NewChunk.Position = OldChunk.Position;

					if (OldChunk.ValuesIndex != -1)
					{
						NewChunk.ValuesIndex = OldChunk.ValuesIndex & (~SingleValueIndexFlag);
						NewChunk.bSingleValue = OldChunk.ValuesIndex & SingleValueIndexFlag;
					}
					if (OldChunk.MaterialsIndex != -1)
					{
						NewChunk.MaterialsIndex = OldChunk.MaterialsIndex & (~SingleValueIndexFlag);
						NewChunk.bSingleMaterial_Unused = OldChunk.MaterialsIndex & SingleValueIndexFlag;
					}
				}
				ensure(NewChunks.GetSlack() == 0);
			}

			// Fixup material indices, as they are now referencing the MaterialsIndices array and not MaterialBuffers/SingleMaterials
			{
				check(OldMaterialBuffers.Num() % VOXELS_PER_DATA_CHUNK == 0);

				MaterialsIndices.Empty(OldMaterialBuffers.Num() / VOXELS_PER_DATA_CHUNK + OldSingleMaterials.Num());
				MaterialBuffers.Empty(OldMaterialBuffers.Num() * FVoxelMaterial::NumChannels);
				SingleMaterials.Empty(OldSingleMaterials.Num() * FVoxelMaterial::NumChannels);

				Chunks.Empty(NewChunks.Num());

				// Fixup chunks
				for (auto& Chunk : NewChunks)
				{
					if (Chunk.MaterialsIndex != -1)
					{
						if (Chunk.bSingleMaterial_Unused)
						{
							const int32 OldIndex = Chunk.MaterialsIndex;

							Chunk.MaterialsIndex = MaterialsIndices.AddUninitialized(1);
							auto& MaterialIndices = MaterialsIndices[Chunk.MaterialsIndex];

							const FVoxelMaterial& Material = OldSingleMaterials[OldIndex];

							for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
							{
								MaterialIndices.GetRaw(Channel) = SingleMaterials.Add(Material.GetRaw(Channel)) | MaterialIndexSingleValueFlag;
							}
						}
						else
						{
							check(Chunk.MaterialsIndex % VOXELS_PER_DATA_CHUNK == 0);

							const int32 OldIndex = Chunk.MaterialsIndex;

							Chunk.MaterialsIndex = MaterialsIndices.AddUninitialized(1);
							auto& MaterialIndices = MaterialsIndices[Chunk.MaterialsIndex];

							for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
							{
								MaterialIndices.GetRaw(Channel) = MaterialBuffers.AddUninitialized(VOXELS_PER_DATA_CHUNK);
							}

							for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
							{
								const FVoxelMaterial& Material = OldMaterialBuffers[OldIndex + Index];

								for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
								{
									MaterialBuffers[MaterialIndices.GetRaw(Channel) + Index] = Material.GetRaw(Channel);
								}
							}
						}
					}
					FVoxelChunkSave NewChunk;
					NewChunk.Position = Chunk.Position;
					NewChunk.ValuesIndex = Chunk.ValuesIndex;
					NewChunk.MaterialsIndex = Chunk.MaterialsIndex;
					NewChunk.bSingleValue = Chunk.bSingleValue;
					Chunks.Add(NewChunk);
				}

				ensure(MaterialsIndices.GetSlack() == 0);
				ensure(MaterialBuffers.GetSlack() == 0);
				ensure(SingleMaterials.GetSlack() == 0);
				ensure(Chunks.GetSlack() == 0);
			}
		}

		// Serialize placeable items
		if (Version >= FVoxelSaveVersion::PlaceableItemsInSave)
		{
			Ar << PlaceableItems;
		}
		
		if (Ar.IsLoading() && Ar.IsError())
		{
			FVoxelMessages::Error("VoxelSave: Serialization failed, data is corrupted");
			*this = FVoxelUncompressedWorldSaveImpl();
		}
		
		UpdateAllocatedSize();
	}

	return true;
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
			Version = FVoxelSaveVersion::LatestVersion;
		}

		Ar << Depth;
		Ar << Version;
		if (Version < FVoxelSaveVersion::ValueConfigFlagAndSaveGUIDs)
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