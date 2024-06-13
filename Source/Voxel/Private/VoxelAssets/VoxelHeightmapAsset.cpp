// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "VoxelAssets/VoxelHeightmapAssetInstance.h"
#include "VoxelIntBox.h"
#include "VoxelMessages.h"
#include "VoxelFeedbackContext.h"
#include "VoxelUtilities/VoxelSerializationUtilities.h"
#include "VoxelGenerators/VoxelEmptyGenerator.h"
#include "VoxelGenerators/VoxelTransformableGeneratorHelper.h"

#include "Serialization/LargeMemoryReader.h"
#include "Serialization/LargeMemoryWriter.h"

#include "Engine/Texture2D.h"
#include "Misc/ScopedSlowTask.h"

#define LANDSCAPE_ASSET_THUMBNAIL_RES 128

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelHeightmapAssetMemory);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<>
VOXEL_API TVoxelHeightmapAssetSamplerWrapper<uint16>::TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset)
	: Scale(Asset ? FMath::Max(SMALL_NUMBER, Asset->Scale) : 1)
	, HeightScale(Asset ? Asset->HeightScale : 1)
	, HeightOffset(Asset ? Asset->HeightOffset : 0)
	, Data(Asset ? CastChecked<UVoxelHeightmapAssetUINT16>(Asset)->GetDataSharedPtr() : MakeVoxelShared<TVoxelHeightmapAssetData<uint16>>())
{
}
template<>
VOXEL_API TVoxelHeightmapAssetSamplerWrapper<float>::TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset)
	: Scale(Asset ? FMath::Max(SMALL_NUMBER, Asset->Scale) : 1)
	, HeightScale(Asset ? Asset->HeightScale : 1)
	, HeightOffset(Asset ? Asset->HeightOffset : 0)
	, Data(Asset ? CastChecked<UVoxelHeightmapAssetFloat>(Asset)->GetDataSharedPtr() : MakeVoxelShared<TVoxelHeightmapAssetData<float>>())
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void UVoxelHeightmapAsset::TryLoad(TVoxelHeightmapAssetData<T>& Data)
{
	if (Data.IsEmpty())
	{
		// Seems invalid, try to load
		LoadData(Data);
	}
}

template<typename T>
void UVoxelHeightmapAsset::SaveData(const TVoxelHeightmapAssetData<T>& Data)
{
	Modify();
	
	FVoxelScopedSlowTask Saving(2.f);

	VoxelCustomVersion = FVoxelHeightmapAssetDataVersion::LatestVersion;
	MaterialConfigFlag = GVoxelMaterialConfigFlag;

	Saving.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing"));

	FLargeMemoryWriter MemoryWriter(Data.GetAllocatedSize());
	
	bool bNeedToSave = false;
	const_cast<TVoxelHeightmapAssetData<T>&>(Data).Serialize(MemoryWriter, MaterialConfigFlag, FVoxelHeightmapAssetDataVersion::Type(VoxelCustomVersion), bNeedToSave);
	ensure(!bNeedToSave);
	
	Saving.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Compressing"));
	FVoxelSerializationUtilities::CompressData(MemoryWriter, CompressedData);

	SyncProperties(Data);

#if WITH_EDITOR
	// Clear thumbnail
	ThumbnailSave.Reset();
	ThumbnailTexture = nullptr;
#endif
}

template<typename T>
void UVoxelHeightmapAsset::LoadData(TVoxelHeightmapAssetData<T>& Data)
{
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

	bool bNeedToSave = false;
	Data.Serialize(MemoryReader, MaterialConfigFlag, FVoxelHeightmapAssetDataVersion::Type(VoxelCustomVersion), bNeedToSave);
	
	if (!ensure(!MemoryReader.IsError()))
	{
		FVoxelMessages::Error("Serialization failed, data is corrupted", this);
		Data.ClearData();
		return;
	}
	ensure(MemoryReader.AtEnd());

	if (bNeedToSave)
	{
		SaveData(Data);
	}

	SyncProperties(Data);
}

template<typename T>
void UVoxelHeightmapAsset::SyncProperties(const TVoxelHeightmapAssetData<T>& Data)
{
	// To access those properties without loading the asset
	Width = Data.GetWidth();
	Height = Data.GetHeight();
}

template<typename T>
FVoxelIntBox UVoxelHeightmapAsset::GetBoundsImpl() const
{
	// const cast to load
	auto Wrapper = TVoxelHeightmapAssetSamplerWrapper<T>(const_cast<UVoxelHeightmapAsset*>(this));

	return FVoxelIntBox(
		FIntVector(
			0,
			0,
			FMath::FloorToInt(-Precision + Wrapper.GetMinHeight() - AdditionalThickness)),
		FIntVector(
			Wrapper.GetWidth(),
			Wrapper.GetHeight(),
			FMath::CeilToInt(Precision + Wrapper.GetMaxHeight()))).Translate(
				FIntVector(
					-Wrapper.GetWidth() / 2,
					-Wrapper.GetHeight() / 2,
					0));
}

void UVoxelHeightmapAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (VoxelCustomVersion == FVoxelHeightmapAssetDataVersion::BeforeCustomVersionWasAdded)
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

#if WITH_EDITOR
template<typename T, typename U>
UTexture2D* UVoxelHeightmapAsset::GetThumbnailInternal()
{
	if (ThumbnailSave.Num() == 0)
	{
		const TVoxelHeightmapAssetData<T>& Data = CastChecked<U>(this)->GetData();
		ThumbnailSave.SetNumUninitialized(LANDSCAPE_ASSET_THUMBNAIL_RES * LANDSCAPE_ASSET_THUMBNAIL_RES);
		for (int32 X = 0; X < LANDSCAPE_ASSET_THUMBNAIL_RES; X++)
		{
			for (int32 Y = 0; Y < LANDSCAPE_ASSET_THUMBNAIL_RES; Y++)
			{
				const float HeightValue = Data.GetHeight(
					float(X) / LANDSCAPE_ASSET_THUMBNAIL_RES * Data.GetWidth(),
					float(Y) / LANDSCAPE_ASSET_THUMBNAIL_RES * Data.GetHeight(),
					EVoxelSamplerMode::Clamp);
				const float Value = (HeightValue - Data.GetMinHeight()) / float(Data.GetMaxHeight() - Data.GetMinHeight());
				const uint8 Byte = FVoxelUtilities::FloatToUINT8(Value);
				ThumbnailSave[X + LANDSCAPE_ASSET_THUMBNAIL_RES * Y] = FColor(Byte, Byte, Byte, 255);
			}
		}
	}
	if (!ThumbnailTexture)
	{
		ThumbnailTexture = UTexture2D::CreateTransient(LANDSCAPE_ASSET_THUMBNAIL_RES, LANDSCAPE_ASSET_THUMBNAIL_RES);
		ThumbnailTexture->CompressionSettings = TC_HDR;
		ThumbnailTexture->SRGB = false;

		ThumbnailSave.SetNumZeroed(LANDSCAPE_ASSET_THUMBNAIL_RES * LANDSCAPE_ASSET_THUMBNAIL_RES);

		FTexture2DMipMap& Mip = ThumbnailTexture->GetPlatformData()->Mips[0];

		void* TextureData = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(TextureData, ThumbnailSave.GetData(), ThumbnailSave.Num() * sizeof(FColor));

		Mip.BulkData.Unlock();
		ThumbnailTexture->UpdateResource();
	}

	return ThumbnailTexture;
}

UTexture2D* UVoxelHeightmapAsset::GetThumbnail()
{
	if (IsA<UVoxelHeightmapAssetFloat>())
	{
		return GetThumbnailInternal<float, UVoxelHeightmapAssetFloat>();
	}
	else if (IsA<UVoxelHeightmapAssetUINT16>())
	{
		return GetThumbnailInternal<uint16, UVoxelHeightmapAssetUINT16>();
	}
	else
	{
		check(false);
		return nullptr;
	}
}
#endif

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelHeightmapAssetFloat::UVoxelHeightmapAssetFloat()
	: Data(MakeVoxelShared<TVoxelHeightmapAssetData<float>>())
{
	HeightScale = 0.01f;
}

TVoxelHeightmapAssetData<float>& UVoxelHeightmapAssetFloat::GetData()
{
	return *GetDataSharedPtr();
}

TVoxelSharedRef<TVoxelHeightmapAssetData<float>> UVoxelHeightmapAssetFloat::GetDataSharedPtr()
{
	TryLoad(*Data);
	return Data.ToSharedRef();
}

void UVoxelHeightmapAssetFloat::Save()
{
	SaveData(*Data);
}

TVoxelSharedRef<TVoxelHeightmapAssetInstance<float>> UVoxelHeightmapAssetFloat::GetInstanceImpl()
{
	return MakeVoxelShared<TVoxelHeightmapAssetInstance<float>>(*this);
}

TVoxelSharedRef<FVoxelGeneratorInstance> UVoxelHeightmapAssetFloat::GetInstance()
{
	return GetInstanceImpl();
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelHeightmapAssetFloat::GetTransformableInstance()
{
	return MakeVoxelShared<TVoxelTransformableGeneratorHelper<TVoxelHeightmapAssetInstance<float>>>(GetInstanceImpl(), false);
}

FVoxelIntBox UVoxelHeightmapAssetFloat::GetBounds() const
{
	return GetBoundsImpl<float>();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelHeightmapAssetUINT16::UVoxelHeightmapAssetUINT16()
	: Data(MakeVoxelShared<TVoxelHeightmapAssetData<uint16>>())
{
	HeightOffset = -32768 / 128.f;
	HeightScale = 1.f / 128.f;
	AdditionalThickness = -HeightOffset;
}

TVoxelHeightmapAssetData<uint16>& UVoxelHeightmapAssetUINT16::GetData()
{
	return *GetDataSharedPtr();
}

TVoxelSharedRef<TVoxelHeightmapAssetData<uint16>> UVoxelHeightmapAssetUINT16::GetDataSharedPtr()
{
	TryLoad(*Data);
	return Data.ToSharedRef();
}

void UVoxelHeightmapAssetUINT16::Save()
{
	SaveData(*Data);
}

TVoxelSharedRef<TVoxelHeightmapAssetInstance<uint16>> UVoxelHeightmapAssetUINT16::GetInstanceImpl()
{
	return MakeVoxelShared<TVoxelHeightmapAssetInstance<uint16>>(*this);
}

TVoxelSharedRef<FVoxelGeneratorInstance> UVoxelHeightmapAssetUINT16::GetInstance()
{
	return GetInstanceImpl();
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> UVoxelHeightmapAssetUINT16::GetTransformableInstance()
{
	return MakeVoxelShared<TVoxelTransformableGeneratorHelper<TVoxelHeightmapAssetInstance<uint16>>>(GetInstanceImpl(), false);
}

FVoxelIntBox UVoxelHeightmapAssetUINT16::GetBounds() const
{
	return GetBoundsImpl<uint16>();
}