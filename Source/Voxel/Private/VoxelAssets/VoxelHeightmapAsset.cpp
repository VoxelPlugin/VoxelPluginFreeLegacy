// Copyright 2020 Phyronnaz

#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "IntBox.h"
#include "VoxelSerializationUtilities.h"
#include "VoxelCustomVersion.h"
#include "VoxelMessages.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"
#include "VoxelData/VoxelTransformableWorldGeneratorHelper.h"

#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

#include "Engine/Texture2D.h"
#include "Misc/ScopedSlowTask.h"

#define LANDSCAPE_ASSET_THUMBNAIL_RES 128

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelHeightmapAssetMemory);

template<typename T>
struct TVoxelHeightmapAssetSelector;

template<>
struct TVoxelHeightmapAssetSelector<float>
{
	using Type = UVoxelHeightmapAssetFloat;
};

template<>
struct TVoxelHeightmapAssetSelector<uint16>
{
	using Type = UVoxelHeightmapAssetUINT16;
};

template<typename T>
class VOXEL_API TVoxelHeightmapAssetInstance : public TVoxelWorldGeneratorInstanceHelper<TVoxelHeightmapAssetInstance<T>, typename TVoxelHeightmapAssetSelector<T>::Type>
{
public:
	const TVoxelHeightmapAssetSamplerWrapper<T> Wrapper;
	const float Precision;
	const bool bInfiniteExtent;
	const FIntBox WorldBounds;

public:
	TVoxelHeightmapAssetInstance(
		const TVoxelHeightmapAssetSamplerWrapper<T>& Wrapper,
		float Precision,
		bool bInfiniteExtent,
		const FIntBox& WorldBounds)
		: Wrapper(Wrapper)
		, Precision(Precision)
		, bInfiniteExtent(bInfiniteExtent)
		, WorldBounds(WorldBounds)
	{
	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
	inline v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (bInfiniteExtent || WorldBounds.ContainsFloat(X, Y, Z)) // Note: it's safe to access outside the bounds
		{
			const float Height = Wrapper.GetHeight(X + Wrapper.GetWidth() / 2, Y + Wrapper.GetHeight() / 2, EVoxelSamplerMode::Clamp);
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
		return Wrapper.GetMaterial(X + Wrapper.GetWidth() / 2, Y + Wrapper.GetHeight() / 2, EVoxelSamplerMode::Clamp);
	}
	inline TVoxelRange<v_flt> GetValueRangeImpl(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (bInfiniteExtent || Bounds.Intersect(WorldBounds))
		{
			const auto ZRange = TVoxelRange<v_flt>(Bounds.Min.Z, Bounds.Max.Z);
			const auto HeightRange = TVoxelRange<v_flt>(Wrapper.GetMinHeight(), Wrapper.GetMaxHeight());
			const auto Range = (ZRange - HeightRange) / Precision;
			if (bInfiniteExtent || WorldBounds.Contains(Bounds))
			{
				return Range;
			}
			else
			{
				return TVoxelRange<v_flt>::Union(1.f, Range); // Intersects with the boundary
			}
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
				const float Height = Wrapper.GetHeight(X + Wrapper.GetWidth() / 2, Y + Wrapper.GetHeight() / 2, EVoxelSamplerMode::Clamp);

				for (VOXEL_QUERY_ZONE_ITERATE(QueryZone, Z))
				{
					FVoxelValue Value;
					if (bInfiniteExtent || WorldBounds.Contains(X, Y, Z))
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
void TVoxelHeightmapAssetData<T>::Serialize(FArchive& Ar, uint32 MaterialConfigFlag, int32 VoxelCustomVersion)
{
	FVoxelScopedSlowTask Serializing(2.f);
	
	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing heights"));
	if (VoxelCustomVersion == FVoxelCustomVersion::BeforeCustomVersionWasAdded)
	{
		Ar << Heights;
	}
	else
	{
		Heights.BulkSerialize(Ar);
	}

	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing materials"));
	if (VoxelCustomVersion < FVoxelCustomVersion::NoVoxelMaterialInHeightmapAssets)
	{
		TArray<FVoxelMaterial> LegacyMaterials;
		FVoxelSerializationUtilities::SerializeMaterials(Ar, LegacyMaterials, MaterialConfigFlag, VoxelCustomVersion);

		// Assume RGB
		Materials.Reserve(LegacyMaterials.Num() * 4);
		for (auto& Material : LegacyMaterials)
		{
			const FColor Color = Material.GetColor();
			Materials.Add(Color.R);
			Materials.Add(Color.G);
			Materials.Add(Color.B);
			Materials.Add(Color.A);
		}
	}
	else if (VoxelCustomVersion < FVoxelCustomVersion::FixMissingMaterialsInHeightmapAssets)
	{
		// Do nothing
	}
	else
	{
		Materials.BulkSerialize(Ar);
	}
	
	Ar << Width;
	Ar << Height;
	Ar << MaxHeight;
	Ar << MinHeight;

	if (VoxelCustomVersion >= FVoxelCustomVersion::NoVoxelMaterialInHeightmapAssets)
	{
		Ar << MaterialConfig;
	}

	if (Width * Height != Heights.Num())
	{
		Ar.SetError();
	}

	if (Materials.Num() > 0)
	{
		int32 MaterialSize = 0;
		switch (MaterialConfig)
		{
		case EVoxelMaterialConfig::RGB:
			MaterialSize = 4;
			break;
		case EVoxelMaterialConfig::SingleIndex:
			MaterialSize = 1;
			break;
		case EVoxelMaterialConfig::DoubleIndex:
			MaterialSize = 3;
			break;
		default:
			Ar.SetError();
		}

		if (MaterialSize * Width * Height != Materials.Num())
		{
			Ar.SetError();
		}
	}

	UpdateStats();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<>
VOXEL_API TVoxelHeightmapAssetSamplerWrapper<uint16>::TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset)
	: Scale(Asset ? FMath::Max(SMALL_NUMBER, Asset->Scale) : 1)
	, HeightScale(Asset ? Asset->HeightScale : 1)
	, HeightOffset(Asset ? Asset->HeightOffset : 0)
	, Data(Asset ? CastChecked<UVoxelHeightmapAssetUINT16>(Asset)->GetDataSharedPtr() : MakeVoxelShared<TVoxelHeightmapAssetData<uint16>>(nullptr))
{
}
template<>
VOXEL_API TVoxelHeightmapAssetSamplerWrapper<float>::TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset)
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
	
	FVoxelScopedSlowTask Saving(2.f);

	VoxelCustomVersion = FVoxelCustomVersion::LatestVersion;
	MaterialConfigFlag = GVoxelMaterialConfigFlag;

	Saving.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing"));

	FBufferArchive Archive(true);
	Data->Serialize(Archive, MaterialConfigFlag, VoxelCustomVersion);

	Saving.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Compressing"));
	FVoxelSerializationUtilities::CompressData(Archive, CompressedData);

	SyncProperties(Data);

#if WITH_EDITOR
	// Clear thumbnail
	ThumbnailSave.Reset();
	ThumbnailTexture = nullptr;
#endif
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
		FVoxelMessages::Error("Decompression failed, data is corrupted", this);
		return;
	}

	FMemoryReader Reader(UncompressedData);
	Data->Serialize(Reader, MaterialConfigFlag, VoxelCustomVersion);
	if (Reader.IsError())
	{
		FVoxelMessages::Error("Serialization failed, data is corrupted", this);
		Data->SetSize(0, 0, false, EVoxelMaterialConfig::RGB);
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

template<typename T>
FIntBox UVoxelHeightmapAsset::GetBoundsImpl() const
{
	// const cast to load
	auto Wrapper = TVoxelHeightmapAssetSamplerWrapper<T>(const_cast<UVoxelHeightmapAsset*>(this));

	return FIntBox(
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
	return *GetDataSharedPtr();
}

TVoxelSharedRef<TVoxelHeightmapAssetData<float>> UVoxelHeightmapAssetFloat::GetDataSharedPtr()
{
	TryLoad(Data);
	return Data;
}

void UVoxelHeightmapAssetFloat::Save()
{
	SaveData(Data);
}

TVoxelSharedRef<TVoxelHeightmapAssetInstance<float>> UVoxelHeightmapAssetFloat::GetInstanceImpl()
{
	return MakeVoxelShared<TVoxelHeightmapAssetInstance<float>>(
		TVoxelHeightmapAssetSamplerWrapper<float>(this),
		Precision,
		bInfiniteExtent,
		GetBounds());
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelHeightmapAssetFloat::GetInstance()
{
	return GetInstanceImpl();
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelHeightmapAssetFloat::GetTransformableInstance()
{
	return MakeVoxelShared<TVoxelTransformableWorldGeneratorHelper<TVoxelHeightmapAssetInstance<float>>>(GetInstanceImpl(), false);
}

FIntBox UVoxelHeightmapAssetFloat::GetBounds() const
{
	return GetBoundsImpl<float>();
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
	return *GetDataSharedPtr();
}

TVoxelSharedRef<TVoxelHeightmapAssetData<uint16>> UVoxelHeightmapAssetUINT16::GetDataSharedPtr()
{
	TryLoad(Data);
	return Data;
}

void UVoxelHeightmapAssetUINT16::Save()
{
	SaveData(Data);
}

TVoxelSharedRef<TVoxelHeightmapAssetInstance<uint16>> UVoxelHeightmapAssetUINT16::GetInstanceImpl()
{
	return MakeVoxelShared<TVoxelHeightmapAssetInstance<uint16>>(
		TVoxelHeightmapAssetSamplerWrapper<uint16>(this),
		Precision,
		bInfiniteExtent,
		GetBounds());
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelHeightmapAssetUINT16::GetInstance()
{
	return GetInstanceImpl();
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelHeightmapAssetUINT16::GetTransformableInstance()
{
	return MakeVoxelShared<TVoxelTransformableWorldGeneratorHelper<TVoxelHeightmapAssetInstance<uint16>>>(GetInstanceImpl(), false);
}

FIntBox UVoxelHeightmapAssetUINT16::GetBounds() const
{
	return GetBoundsImpl<uint16>();
}