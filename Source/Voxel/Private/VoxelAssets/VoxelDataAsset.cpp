// Copyright 2020 Phyronnaz

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.h"
#include "VoxelAssets/VoxelDataAssetInstance.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"
#include "VoxelWorldGenerators/VoxelTransformableWorldGeneratorHelper.h"
#include "VoxelUtilities/VoxelSerializationUtilities.h"
#include "VoxelFeedbackContext.h"
#include "VoxelMessages.h"

#include "Engine/Texture2D.h"
#include "Serialization/LargeMemoryReader.h"
#include "Serialization/LargeMemoryWriter.h"

UVoxelDataAsset::UVoxelDataAsset()
{
	Data = MakeVoxelShared<FVoxelDataAssetData>();
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelDataAsset::GetInstance()
{
	return GetInstanceImpl();
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelDataAsset::GetTransformableInstance()
{
	return MakeVoxelShared<TVoxelTransformableWorldGeneratorHelper<FVoxelDataAssetInstance>>(GetInstanceImpl(), bSubtractiveAsset);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<const FVoxelDataAssetData> UVoxelDataAsset::GetData()
{
	TryLoad();
	return Data.ToSharedRef();
}

void UVoxelDataAsset::SetData(const TVoxelSharedRef<FVoxelDataAssetData>& InData)
{
	Data = InData;
	Save();
}

TVoxelSharedRef<FVoxelDataAssetInstance> UVoxelDataAsset::GetInstanceImpl()
{
	TryLoad();
	return MakeVoxelShared<FVoxelDataAssetInstance>(*this);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataAsset::Save()
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelScopedSlowTask Saving(2.f);

	Modify();

	VoxelCustomVersion = FVoxelDataAssetDataVersion::LatestVersion;
	ValueConfigFlag = GVoxelValueConfigFlag;
	MaterialConfigFlag = GVoxelMaterialConfigFlag;

	Saving.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing"));

	FLargeMemoryWriter MemoryWriter(Data->GetAllocatedSize());
	Data->Serialize(MemoryWriter, ValueConfigFlag, MaterialConfigFlag, FVoxelDataAssetDataVersion::Type(VoxelCustomVersion));

	Saving.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Compressing"));
	FVoxelSerializationUtilities::CompressData(MemoryWriter, CompressedData);

	SyncProperties();
}


void UVoxelDataAsset::Load()
{
	VOXEL_FUNCTION_COUNTER();
	
	if (CompressedData.Num() == 0)
	{
		// Nothing to load
		return;
	}
	
	TArray64<uint8> UncompressedData;
	if (!FVoxelSerializationUtilities::DecompressData(CompressedData, UncompressedData))
	{
		FVoxelMessages::Error("Decompression failed, data is corrupted", this);
		return;
	}

	FLargeMemoryReader MemoryReader(UncompressedData.GetData(), UncompressedData.Num());
	Data->Serialize(MemoryReader, ValueConfigFlag, MaterialConfigFlag, FVoxelDataAssetDataVersion::Type(VoxelCustomVersion));
	
	if (!ensure(!MemoryReader.IsError()))
	{
		FVoxelMessages::Error("Serialization failed, data is corrupted", this);
	}
	ensure(MemoryReader.AtEnd());

	SyncProperties();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataAsset::TryLoad()
{
	if (Data->IsEmpty())
	{
		// Seems invalid, try to load
		Load();
	}
}

void UVoxelDataAsset::SyncProperties()
{
	// To access those properties without loading the asset
	Size = Data->GetSize();
	UncompressedSizeInMB =
		Data->GetRawValues().Num() * sizeof(FVoxelValue) / double(1 << 20) +
		Data->GetRawMaterials().Num() * sizeof(FVoxelMaterial) / double(1 << 20);
	CompressedSizeInMB = CompressedData.Num() / double(1 << 20);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataAsset::Serialize(FArchive& Ar)
{
	VOXEL_FUNCTION_COUNTER();
	
	Super::Serialize(Ar);

	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (VoxelCustomVersion == FVoxelDataAssetDataVersion::BeforeCustomVersionWasAdded)
		{
			Ar << MaterialConfigFlag;
			Ar << CompressedData;
		}
		else
		{
			CompressedData.BulkSerialize(Ar);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#if WITH_EDITOR
void UVoxelDataAsset::SetThumbnail(TArray<FColor>&& Colors)
{
	VOXEL_FUNCTION_COUNTER();
	
	ThumbnailSave = MoveTemp(Colors);
	ThumbnailTexture = nullptr;

	if (ThumbnailSave.Num() != Colors.Num())
	{
		MarkPackageDirty();
		return;
	}

	for (int32 Index = 0; Index < Colors.Num(); Index++)
	{
		if (ThumbnailSave[Index] != Colors[Index])
		{
			MarkPackageDirty();
			return;
		}
	}
}

UTexture2D* UVoxelDataAsset::GetThumbnail()
{
	if (!ThumbnailTexture)
	{
		ThumbnailTexture = UTexture2D::CreateTransient(DATA_ASSET_THUMBNAIL_RES, DATA_ASSET_THUMBNAIL_RES);
		ThumbnailTexture->CompressionSettings = TC_HDR;
		ThumbnailTexture->SRGB = false;

		ThumbnailSave.SetNumZeroed(DATA_ASSET_THUMBNAIL_RES * DATA_ASSET_THUMBNAIL_RES);

		FTexture2DMipMap& Mip = ThumbnailTexture->PlatformData->Mips[0];

		void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, ThumbnailSave.GetData(), ThumbnailSave.Num() * sizeof(FColor));

		Mip.BulkData.Unlock();
		ThumbnailTexture->UpdateResource();
	}

	return ThumbnailTexture;
}
#endif