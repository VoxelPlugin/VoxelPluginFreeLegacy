// Copyright 2020 Phyronnaz

#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "IntBox.h"
#include "VoxelSerializationUtilities.h"
#include "VoxelCustomVersion.h"
#include "VoxelMessages.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"

#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

#include "Engine/Texture2D.h"
#include "Misc/ScopedSlowTask.h"

#define LOCTEXT_NAMESPACE "Voxel"

#define LANDSCAPE_ASSET_THUMBNAIL_RES 128

DEFINE_STAT(STAT_VoxelHeightmapAssetMemory);

template<typename T>
struct UVoxelHeightmapAssetSelector;

template<>
struct UVoxelHeightmapAssetSelector<float>
{
	inline static UClass* StaticClass() { return UVoxelHeightmapAssetFloat::StaticClass(); }
};

template<>
struct UVoxelHeightmapAssetSelector<uint16>
{
	inline static UClass* StaticClass() { return UVoxelHeightmapAssetUINT16::StaticClass(); }
};

template<typename T>
class VOXEL_API TVoxelLandscapeAssetInstance : public TVoxelWorldGeneratorInstanceHelper<TVoxelLandscapeAssetInstance<T>, UVoxelHeightmapAssetSelector<T>>
{
public:
	const TVoxelHeightmapAssetSamplerWrapper<T> Wrapper;
	const float Precision;
	const float AdditionalThickness;
	const FIntBox WorldBounds;

	inline FIntPoint GetCenter() const
	{
		return FIntPoint(Wrapper.GetWidth() / 2, Wrapper.GetHeight() / 2);
	}

public:
	TVoxelLandscapeAssetInstance(
		const TVoxelHeightmapAssetSamplerWrapper<T>& Wrapper,
		float Precision,
		float AdditionalThickness)
		: Wrapper(Wrapper)
		, Precision(Precision)
		, AdditionalThickness(AdditionalThickness)
		, WorldBounds(
			FIntVector(
				GetCenter().X - Wrapper.GetWidth(),
				GetCenter().Y - Wrapper.GetHeight(),
				FMath::FloorToInt(-Precision + Wrapper.GetMinHeight() - AdditionalThickness)),
			FIntVector(
				GetCenter().X,
				GetCenter().Y,
				FMath::CeilToInt(Precision + Wrapper.GetMaxHeight())))
	{
	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
	inline v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (WorldBounds.ContainsFloat(X, Y, Z)) // Note: it's safe to access outside the bounds
		{
			const float Height = Wrapper.GetHeight(X + GetCenter().X, Y + GetCenter().Y, EVoxelSamplerMode::Clamp);
			return (Z - Height) / Precision;
		}
		else
		{
			// Outside asset bounds
			return 1;
		}
	}
	inline FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		return Wrapper.GetMaterial(X + GetCenter().X, Y + GetCenter().Y, EVoxelSamplerMode::Clamp);
	}
	inline TVoxelRange<v_flt> GetValueRangeImpl(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (Bounds.Intersect(WorldBounds))
		{
			const auto ZRange = TVoxelRange<v_flt>(Bounds.Min.Z, Bounds.Max.Z);
			const auto HeightRange = TVoxelRange<v_flt>(Wrapper.GetMinHeight(), Wrapper.GetMaxHeight());
			return (ZRange - HeightRange) / Precision;
		}
		else
		{
			return 1;
		}
	}
	virtual void GetValues(TVoxelQueryZone<FVoxelValue>& QueryZone, int32 LOD, const FVoxelItemStack& Items) const override final
	{
		for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, X))
		{
			for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Y))
			{
				const float Height = Wrapper.GetHeight(X + GetCenter().X, Y + GetCenter().Y, EVoxelSamplerMode::Clamp);

				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					FVoxelValue Value;
					if (WorldBounds.Contains(X, Y, Z))
					{
						Value = FVoxelValue((Z - Height) / Precision);
					}
					else
					{
						// Outside asset bounds
						Value = FVoxelValue::Empty();
					}
					QueryZone.Set(X, Y, Z, Value);
				}
			}
		}
	}
	virtual FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelWorldGeneratorInstance Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void TVoxelHeightmapAssetData<T>::SerializeAsset(FArchive& Ar, uint32 MaterialConfigFlag, int32 VoxelCustomVersion)
{
	FScopedSlowTask Serializing(2.f);
	
	Serializing.EnterProgressFrame(1.f, LOCTEXT("SerializingHeights", "Serializing heights"));
	if (VoxelCustomVersion == FVoxelCustomVersion::BeforeCustomVersionWasAdded)
	{
		Ar << Heights;
	}
	else
	{
		Heights.BulkSerialize(Ar);
	}

	Serializing.EnterProgressFrame(1.f, LOCTEXT("SerializingMaterials", "Serializing materials"));
	FVoxelSerializationUtilities::SerializeMaterials(Ar, Materials, MaterialConfigFlag, VoxelCustomVersion);
	
	Ar << Width;
	Ar << Height;
	Ar << MaxHeight;
	Ar << MinHeight;

	if (Width * Height != Heights.Num() || (Materials.Num() > 0 && Width * Height != Materials.Num()))
	{
		Ar.SetError();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<>
TVoxelHeightmapAssetSamplerWrapper<uint16>::TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset)
	: Scale(Asset ? FMath::Max(SMALL_NUMBER, Asset->Scale) : 1)
	, HeightScale(Asset ? Asset->HeightScale : 1)
	, HeightOffset(Asset ? Asset->HeightOffset : 0)
	, Data(Asset ? CastChecked<UVoxelHeightmapAssetUINT16>(Asset)->GetDataSharedPtr() : MakeVoxelShared<TVoxelHeightmapAssetData<uint16>>(nullptr))
{
}
template<>
TVoxelHeightmapAssetSamplerWrapper<float>::TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset)
	: Scale(Asset ? FMath::Max(SMALL_NUMBER, Asset->Scale) : 1)
	, HeightScale(Asset ? Asset->HeightScale : 1)
	, HeightOffset(Asset ? Asset->HeightOffset : 0)
	, Data(Asset ? CastChecked<UVoxelHeightmapAssetFloat>(Asset)->GetDataSharedPtr() : MakeVoxelShared<TVoxelHeightmapAssetData<float>>(nullptr))
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void UVoxelHeightmapAsset::TryLoad(TVoxelSharedRef<TVoxelHeightmapAssetData<T>>& Data)
{
	if (Data->IsEmpty())
	{
		// Seems invalid, try to load
		LoadData(Data);
	}
}

template<typename T>
void UVoxelHeightmapAsset::SaveData(const TVoxelSharedRef<TVoxelHeightmapAssetData<T>>& Data)
{
	Modify();
	
	FScopedSlowTask Saving(2.f);

	VoxelCustomVersion = FVoxelCustomVersion::LatestVersion;
	MaterialConfigFlag = GVoxelMaterialConfigFlag;

	Saving.EnterProgressFrame(1.f, LOCTEXT("Serializing", "Serializing"));

	FBufferArchive Archive(true);
	Data->SerializeAsset(Archive, MaterialConfigFlag, VoxelCustomVersion);

	Saving.EnterProgressFrame(1.f, LOCTEXT("Compressing", "Compressing"));
	FVoxelSerializationUtilities::CompressData(Archive, CompressedData);

	SyncProperties(Data);
}

template<typename T>
void UVoxelHeightmapAsset::LoadData(TVoxelSharedRef<TVoxelHeightmapAssetData<T>>& Data)
{
	if (CompressedData.Num() == 0)
	{
		// Nothing to load
		return;
	}
	TArray<uint8> UncompressedData;
	if (!FVoxelSerializationUtilities::DecompressData(CompressedData, UncompressedData))
	{
		FVoxelMessages::Error(NSLOCTEXT("Voxel", "VoxelHeightmapAssetDecompressionFailed", "Decompression failed, data is corrupted"), this);
		return;
	}

	FMemoryReader Reader(UncompressedData);
	Data->SerializeAsset(Reader, MaterialConfigFlag, VoxelCustomVersion);
	if (Reader.IsError())
	{
		FVoxelMessages::Error(NSLOCTEXT("Voxel", "VoxelHeightmapAssetReaderError", "Serialization failed, data is corrupted"), this);
		Data->SetSize(0, 0, false);
	}

	SyncProperties(Data);
}

template<typename T>
void UVoxelHeightmapAsset::SyncProperties(const TVoxelSharedRef<TVoxelHeightmapAssetData<T>>& Data)
{
	// To access those properties without loading the asset
	Width = Data->GetWidth();
	Height = Data->GetHeight();
}

void UVoxelHeightmapAsset::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (VoxelCustomVersion == FVoxelCustomVersion::BeforeCustomVersionWasAdded)
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
					EVoxelSamplerMode::Clamp, 
					Data.GetMinHeight());
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

		FTexture2DMipMap& Mip = ThumbnailTexture->PlatformData->Mips[0];

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
{
	HeightScale = 0.01f;
}

TVoxelHeightmapAssetData<float>& UVoxelHeightmapAssetFloat::GetData()
{
	TryLoad(Data);
	return *Data;
}

TVoxelSharedRef<const TVoxelHeightmapAssetData<float>> UVoxelHeightmapAssetFloat::GetDataSharedPtr()
{
	TryLoad(Data);
	return Data;
}

void UVoxelHeightmapAssetFloat::Save()
{
	SaveData(Data);
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelHeightmapAssetFloat::GetInstance()
{
	return MakeVoxelShared<TVoxelLandscapeAssetInstance<float>>(
		this,
		Precision,
		AdditionalThickness);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelHeightmapAssetUINT16::UVoxelHeightmapAssetUINT16()
{
	HeightOffset = -32768 / 128.f;
	HeightScale = 1.f / 128.f;
	AdditionalThickness = -HeightOffset;
}

TVoxelHeightmapAssetData<uint16>& UVoxelHeightmapAssetUINT16::GetData()
{
	TryLoad(Data);
	return *Data;
}

TVoxelSharedRef<const TVoxelHeightmapAssetData<uint16>> UVoxelHeightmapAssetUINT16::GetDataSharedPtr()
{
	TryLoad(Data);
	return Data;
}

void UVoxelHeightmapAssetUINT16::Save()
{
	SaveData(Data);
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelHeightmapAssetUINT16::GetInstance()
{
	return MakeVoxelShared<TVoxelLandscapeAssetInstance<uint16>>(
		this,
		Precision,
		AdditionalThickness);
}
#undef LOCTEXT_NAMESPACE