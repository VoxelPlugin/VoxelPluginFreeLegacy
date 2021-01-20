// Copyright 2021 Phyronnaz

#include "VoxelData/VoxelSaveUtilities.h"
#include "VoxelData/VoxelDataOctreeLeafData.h"
#include "VoxelPlaceableItems/VoxelPlaceableItem.h"
#include "VoxelMessages.h"
#include "VoxelUtilities/VoxelSerializationUtilities.h"

#include "Serialization/LargeMemoryReader.h"
#include "Serialization/LargeMemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

FVoxelSaveBuilder::FVoxelSaveBuilder(int32 Depth)
	: Depth(Depth)
{
}

void FVoxelSaveBuilder::Save(FVoxelUncompressedWorldSaveImpl& OutSave, TArray<FVoxelObjectArchiveEntry>& OutObjects)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	check(Depth >= 0);
	OutSave.Guid = FGuid::NewGuid();
	OutSave.Depth = Depth;
	OutSave.Chunks64.Empty(ChunksToSave.Num());

	{
		uint32 NumValueBuffers = 0;
		uint32 NumSingleValues = 0;
		
		uint32 NumMaterialsIndices = 0;
		uint32 NumMaterialBuffers = 0;
		uint32 NumSingleMaterials = 0;
		
		for (auto& Chunk : ChunksToSave)
		{
			if (Chunk.Values->IsDirty())
			{
				NumValueBuffers += Chunk.Values->DataPtr != nullptr;
				NumSingleValues += Chunk.Values->DataPtr == nullptr;
			}

			if (Chunk.Materials->IsDirty())
			{
				NumMaterialsIndices++;
				if (Chunk.Materials->bUseChannels)
				{
					for (auto& DataPtr : Chunk.Materials->Channels_DataPtr)
					{
						NumMaterialBuffers += DataPtr != nullptr;
						NumSingleMaterials += DataPtr == nullptr;
					}
				}
				else
				{
					NumMaterialBuffers += FVoxelMaterial::NumChannels;
				}
			}
		}
		
		OutSave.ValueBuffers64.Empty(NumValueBuffers * VOXELS_PER_DATA_CHUNK);
		OutSave.SingleValues64.Empty(NumSingleValues);
		
		OutSave.MaterialsIndices64.Empty(NumMaterialsIndices);
		OutSave.MaterialBuffers64.Empty(NumMaterialBuffers * VOXELS_PER_DATA_CHUNK);
		OutSave.SingleMaterials64.Empty(NumSingleMaterials);
	}

	for (auto& Chunk : ChunksToSave)
	{
		FVoxelUncompressedWorldSaveImpl::FVoxelChunkSave NewChunk;
		NewChunk.Position = Chunk.Position;
		
		if (Chunk.Values->IsDirty())
		{
			if (Chunk.Values->DataPtr)
			{
				NewChunk.ValuesIndex = OutSave.ValueBuffers64.AddUninitialized(VOXELS_PER_DATA_CHUNK);

#if ONE_BIT_VOXEL_VALUE
				static_assert(TVoxelStaticBitArray<VOXELS_PER_DATA_CHUNK>::NumBitsPerWord == 32, "");
				check(NewChunk.ValuesIndex % 32 == 0);
				FMemory::Memcpy(OutSave.ValueBuffers64.GetWordData() + NewChunk.ValuesIndex / 32, Chunk.Values->DataPtr->GetWordData(), sizeof(TVoxelValueStaticArray<VOXELS_PER_DATA_CHUNK>));
#else
				FMemory::Memcpy(&OutSave.ValueBuffers64[NewChunk.ValuesIndex], Chunk.Values->DataPtr, sizeof(TVoxelValueStaticArray<VOXELS_PER_DATA_CHUNK>));
#endif
			}
			else
			{
				check(Chunk.Values->bIsSingleValue);
				
				NewChunk.ValuesIndex = OutSave.SingleValues64.Add(Chunk.Values->SingleValue);
				NewChunk.bSingleValue = true;
			}
		}
		
		if (Chunk.Materials->IsDirty())
		{
			TVoxelMaterialStorage<uint32> MaterialIndices;
			
			if (Chunk.Materials->bUseChannels)
			{
				for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
				{
					if (auto& DataPtr = Chunk.Materials->Channels_DataPtr[Channel])
					{
						const int32 Index = OutSave.MaterialBuffers64.AddUninitialized(VOXELS_PER_DATA_CHUNK);
						FMemory::Memcpy(&OutSave.MaterialBuffers64[Index], DataPtr, sizeof(uint8) * VOXELS_PER_DATA_CHUNK);

						MaterialIndices.GetRaw(Channel) = Index;
					}
					else
					{
						MaterialIndices.GetRaw(Channel) = OutSave.SingleMaterials64.Add(Chunk.Materials->Channels_SingleValue[Channel]);
						MaterialIndices.GetRaw(Channel) |= FVoxelUncompressedWorldSaveImpl::MaterialIndexSingleValueFlag;
					}
				}
			}
			else
			{
				check(Chunk.Materials->Main_DataPtr);
				
				for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
				{
					MaterialIndices.GetRaw(Channel) = OutSave.MaterialBuffers64.AddUninitialized(VOXELS_PER_DATA_CHUNK);
				}

				for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
				{
					const FVoxelMaterial& Material = Chunk.Materials->Main_DataPtr[Index];
					
					for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
					{
						OutSave.MaterialBuffers64[MaterialIndices.GetRaw(Channel) + Index] = Material.GetRaw(Channel);
					}
				}
			}
			
			NewChunk.MaterialsIndex = OutSave.MaterialsIndices64.Add(MaterialIndices); 
		}

		if (NewChunk.ValuesIndex != -1 || NewChunk.MaterialsIndex != -1)
		{
			OutSave.Chunks64.Add(NewChunk);
		}
	}

	ChunksToSave.Empty();

	// TODO 64 bit
	TArray<uint8> PlaceableItems;
	FMemoryWriter Writer(PlaceableItems);
	{
		FVoxelObjectArchive Archive = FVoxelObjectArchive::MakeWriter(Writer);
		FVoxelPlaceableItemsUtilities::SerializeItems(Archive, {}, AssetItems);
		OutObjects = Archive.GetWriterObjects();
	}
	
	OutSave.PlaceableItems64 = MoveTemp(PlaceableItems);
	OutSave.PlaceableItems64.Shrink();

	OutSave.UpdateAllocatedSize();
}

void FVoxelSaveBuilder::AddAssetItem(const FVoxelAssetItem& AssetItem)
{
	AssetItems.Add(AssetItem);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelSaveLoader::ExtractChunk(
	int32 ChunkIndex,
	const IVoxelDataOctreeMemory& Memory,
	TVoxelDataOctreeLeafData<FVoxelValue>& OutValues,
	TVoxelDataOctreeLeafData<FVoxelMaterial>& OutMaterials) const
{
	OutValues.ClearData(Memory);
	OutMaterials.ClearData(Memory);
	
	auto& Chunk = Save.Chunks64[ChunkIndex];
	if (Chunk.ValuesIndex >= 0)
	{
		if (Chunk.bSingleValue)
		{
			 OutValues.SetSingleValue(Save.SingleValues64[Chunk.ValuesIndex]);
		}
		else
		{
			OutValues.CreateData(Memory, [&](TVoxelValueStaticArray<VOXELS_PER_DATA_CHUNK>* DataPtr)
			{
				check(Save.ValueBuffers64.Num() >= Chunk.ValuesIndex + VOXELS_PER_DATA_CHUNK);
#if ONE_BIT_VOXEL_VALUE
				static_assert(TVoxelStaticBitArray<VOXELS_PER_DATA_CHUNK>::NumBitsPerWord == 32, "");
				check(Chunk.ValuesIndex % 32 == 0);
				FMemory::Memcpy(DataPtr->GetWordData(), Save.ValueBuffers64.GetWordData() + Chunk.ValuesIndex / 32, sizeof(TVoxelValueStaticArray<VOXELS_PER_DATA_CHUNK>));
#else
				FMemory::Memcpy(DataPtr->GetData(), &Save.ValueBuffers64[Chunk.ValuesIndex], sizeof(TVoxelValueStaticArray<VOXELS_PER_DATA_CHUNK>));
#endif
			});
		}
		OutValues.SetIsDirty(true, Memory);
	}
	if (Chunk.MaterialsIndex >= 0)
	{
		const auto& MaterialIndices = Save.MaterialsIndices64[Chunk.MaterialsIndex];

		bool bHasAnySingleValue = false;
		for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
		{
			if (MaterialIndices.GetRaw(Channel) & FVoxelUncompressedWorldSaveImpl::MaterialIndexSingleValueFlag)
			{
				bHasAnySingleValue = true;
				break;
			}
		}
		
		if (bHasAnySingleValue)
		{
			OutMaterials.bUseChannels = true;
			for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
			{
				const int32 ChannelIndex = MaterialIndices.GetRaw(Channel);
				if (ChannelIndex & FVoxelUncompressedWorldSaveImpl::MaterialIndexSingleValueFlag)
				{
					OutMaterials.Channels_SingleValue[Channel] = Save.SingleMaterials64[ChannelIndex & (~FVoxelUncompressedWorldSaveImpl::MaterialIndexSingleValueFlag)];
				}
				else
				{
					uint8* RESTRICT& DataPtr = OutMaterials.Channels_DataPtr[Channel];
					OutMaterials.Channels_Allocate(DataPtr, Memory);
					
					check(Save.MaterialBuffers64.Num() >= ChannelIndex + VOXELS_PER_DATA_CHUNK);
					FMemory::Memcpy(DataPtr, &Save.MaterialBuffers64[ChannelIndex], sizeof(uint8) * VOXELS_PER_DATA_CHUNK);
				}
			}
		}
		else
		{
			OutMaterials.CreateData(Memory, [&](FVoxelMaterial* RESTRICT DataPtr)
			{
				for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
				{
					for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
					{
						DataPtr[Index].GetRaw(Channel) = Save.MaterialBuffers64[MaterialIndices.GetRaw(Channel) + Index];
					}
				}
			});
		}
		OutMaterials.SetIsDirty(true, Memory);
	}
}

void FVoxelSaveLoader::GetPlaceableItems(const FVoxelPlaceableItemLoadInfo& LoadInfo, TArray<FVoxelAssetItem>& OutAssetItems)
{
	VOXEL_FUNCTION_COUNTER();
	ensure(IsInGameThread());

	FLargeMemoryReader Reader(Save.PlaceableItems64.GetData(), Save.PlaceableItems64.Num());

	if (Save.Version < FVoxelSaveVersion::ProperlySerializePlaceableItemsObjects)
	{
		int32 Num;
		Reader << Num;
		if (Num > 0)
		{
			FVoxelMessages::Error(FString::Printf(TEXT("You had %d voxel assets in your scene. These cannot be loaded anymore. Please contact the dev for a workaround."), Num));
		}
	}
	else
	{
		FVoxelObjectArchive Archive = FVoxelObjectArchive::MakeReader(Reader, LoadInfo.Objects ? *LoadInfo.Objects : TArray<FVoxelObjectArchiveEntry>());
		FVoxelPlaceableItemsUtilities::SerializeItems(Archive, LoadInfo, OutAssetItems);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelSaveUtilities::CompressVoxelSave(const FVoxelUncompressedWorldSave& UncompressedSave, FVoxelCompressedWorldSave& OutCompressedSave)
{
	OutCompressedSave.Objects = UncompressedSave.Objects;
	CompressVoxelSave(UncompressedSave.Const(), OutCompressedSave.NewMutable());
}

void UVoxelSaveUtilities::CompressVoxelSave(const FVoxelUncompressedWorldSaveImpl& UncompressedSave, FVoxelCompressedWorldSaveImpl& OutCompressedSave)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	OutCompressedSave.Depth = UncompressedSave.GetDepth();
	OutCompressedSave.Guid = UncompressedSave.GetGuid();

	FLargeMemoryWriter MemoryWriter(UncompressedSave.GetAllocatedSize());
	const_cast<FVoxelUncompressedWorldSaveImpl&>(UncompressedSave).Serialize(MemoryWriter);
	
	FVoxelSerializationUtilities::CompressData(MemoryWriter, OutCompressedSave.CompressedData);
	
	OutCompressedSave.UpdateAllocatedSize();
}

bool UVoxelSaveUtilities::DecompressVoxelSave(const FVoxelCompressedWorldSave& CompressedSave, FVoxelUncompressedWorldSave& OutUncompressedSave)
{
	OutUncompressedSave.Objects = CompressedSave.Objects;
	return DecompressVoxelSave(CompressedSave.Const(), OutUncompressedSave.NewMutable());
}

bool UVoxelSaveUtilities::DecompressVoxelSave(const FVoxelCompressedWorldSaveImpl& CompressedSave, FVoxelUncompressedWorldSaveImpl& OutUncompressedSave)
{
	VOXEL_FUNCTION_COUNTER();
	
	if (CompressedSave.CompressedData.Num() == 0)
	{
		return false;
	}
	else
	{
		TArray64<uint8> UncompressedData;
		if (!FVoxelSerializationUtilities::DecompressData(CompressedSave.CompressedData, UncompressedData))
		{
			FVoxelMessages::Error("DecompressVoxelSave failed: Corrupted data");
			return false;
		}

		FLargeMemoryReader Reader(UncompressedData.GetData(), UncompressedData.Num());
		OutUncompressedSave.Serialize(Reader);
		ensure(Reader.AtEnd() && !Reader.IsError());

		return true;
	}
}