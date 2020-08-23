// Copyright 2020 Phyronnaz

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
	OutSave.Chunks.Empty(ChunksToSave.Num());

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
		
		OutSave.ValueBuffers.Empty(NumValueBuffers * VOXELS_PER_DATA_CHUNK);
		OutSave.SingleValues.Empty(NumSingleValues);
		
		OutSave.MaterialsIndices.Empty(NumMaterialsIndices);
		OutSave.MaterialBuffers.Empty(NumMaterialBuffers * VOXELS_PER_DATA_CHUNK);
		OutSave.SingleMaterials.Empty(NumSingleMaterials);
	}

	for (auto& Chunk : ChunksToSave)
	{
		FVoxelUncompressedWorldSaveImpl::FVoxelChunkSave NewChunk;
		NewChunk.Position = Chunk.Position;
		
		if (Chunk.Values->IsDirty())
		{
			if (Chunk.Values->DataPtr)
			{
				NewChunk.ValuesIndex = OutSave.ValueBuffers.AddUninitialized(VOXELS_PER_DATA_CHUNK);
				FMemory::Memcpy(&OutSave.ValueBuffers[NewChunk.ValuesIndex], Chunk.Values->DataPtr, sizeof(FVoxelValue) * VOXELS_PER_DATA_CHUNK);
			}
			else
			{
				check(Chunk.Values->bIsSingleValue);
				
				NewChunk.ValuesIndex = OutSave.SingleValues.Add(Chunk.Values->SingleValue);
				NewChunk.bSingleValue = true;
			}
		}
		else
		{
			NewChunk.ValuesIndex = -1;
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
						const int32 Index = OutSave.MaterialBuffers.AddUninitialized(VOXELS_PER_DATA_CHUNK);
						FMemory::Memcpy(&OutSave.MaterialBuffers[Index], DataPtr, sizeof(uint8) * VOXELS_PER_DATA_CHUNK);

						MaterialIndices.GetRaw(Channel) = Index;
					}
					else
					{
						MaterialIndices.GetRaw(Channel) = OutSave.SingleMaterials.Add(Chunk.Materials->Channels_SingleValue[Channel]);
						MaterialIndices.GetRaw(Channel) |= FVoxelUncompressedWorldSaveImpl::MaterialIndexSingleValueFlag;
					}
				}
			}
			else
			{
				check(Chunk.Materials->Main_DataPtr);
				
				for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
				{
					MaterialIndices.GetRaw(Channel) = OutSave.MaterialBuffers.AddUninitialized(VOXELS_PER_DATA_CHUNK);
				}

				for (int32 Index = 0; Index < VOXELS_PER_DATA_CHUNK; Index++)
				{
					const FVoxelMaterial& Material = Chunk.Materials->Main_DataPtr[Index];
					
					for (int32 Channel = 0; Channel < FVoxelMaterial::NumChannels; Channel++)
					{
						OutSave.MaterialBuffers[MaterialIndices.GetRaw(Channel) + Index] = Material.GetRaw(Channel);
					}
				}
			}
			
			NewChunk.MaterialsIndex = OutSave.MaterialsIndices.Add(MaterialIndices); 
		}
		else
		{
			NewChunk.MaterialsIndex = -1;
		}

		OutSave.Chunks.Add(NewChunk);
	}

	ensure(OutSave.Chunks.GetSlack() == 0);
	
	ensure(OutSave.ValueBuffers.GetSlack() == 0);
	ensure(OutSave.MaterialBuffers.GetSlack() == 0);

	ensure(OutSave.SingleValues.GetSlack() == 0);
	ensure(OutSave.SingleMaterials.GetSlack() == 0);

	ensure(OutSave.MaterialsIndices.GetSlack() == 0);

	ChunksToSave.Empty();
	
	FMemoryWriter Writer(OutSave.PlaceableItems);
	{
		FVoxelObjectArchive Archive = FVoxelObjectArchive::MakeWriter(Writer);
		FVoxelPlaceableItemsUtilities::SerializeItems(Archive, {}, AssetItems);
		OutObjects = Archive.GetWriterObjects();
	}
	OutSave.PlaceableItems.Shrink();

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
	
	auto& Chunk = Save.Chunks[ChunkIndex];
	if (Chunk.ValuesIndex >= 0)
	{
		if (Chunk.bSingleValue)
		{
			 OutValues.SetSingleValue(Save.SingleValues[Chunk.ValuesIndex]);
		}
		else
		{
			OutValues.CreateData(Memory, [&](FVoxelValue* RESTRICT DataPtr)
			{
				check(Save.ValueBuffers.Num() >= Chunk.ValuesIndex + VOXELS_PER_DATA_CHUNK);
				FMemory::Memcpy(DataPtr, &Save.ValueBuffers[Chunk.ValuesIndex], sizeof(FVoxelValue) * VOXELS_PER_DATA_CHUNK);
			});
		}
		OutValues.SetIsDirty(true, Memory);
	}
	if (Chunk.MaterialsIndex >= 0)
	{
		const auto& MaterialIndices = Save.MaterialsIndices[Chunk.MaterialsIndex];

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
					OutMaterials.Channels_SingleValue[Channel] = Save.SingleMaterials[ChannelIndex & (~FVoxelUncompressedWorldSaveImpl::MaterialIndexSingleValueFlag)];
				}
				else
				{
					uint8* RESTRICT& DataPtr = OutMaterials.Channels_DataPtr[Channel];
					OutMaterials.Channels_Allocate(DataPtr, Memory);
					
					check(Save.MaterialBuffers.Num() >= ChannelIndex + VOXELS_PER_DATA_CHUNK);
					FMemory::Memcpy(DataPtr, &Save.MaterialBuffers[ChannelIndex], sizeof(uint8) * VOXELS_PER_DATA_CHUNK);
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
						DataPtr[Index].GetRaw(Channel) = Save.MaterialBuffers[MaterialIndices.GetRaw(Channel) + Index];
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

	FMemoryReader Reader(Save.PlaceableItems);

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