// Copyright 2020 Phyronnaz

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"
#include "VoxelWorldGeneratorHelpers.h"
#include "VoxelWorldGeneratorInstance.inl"
#include "VoxelCustomVersion.h"
#include "VoxelMessages.h"
#include "VoxelSerializationUtilities.h"

#include "Engine/Texture2D.h"
#include "Misc/ScopedSlowTask.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

DEFINE_STAT(STAT_VoxelDataAssetMemory);

#define LOCTEXT_NAMESPACE "Voxel"

class FVoxelDataAssetInstance : public TVoxelTransformableWorldGeneratorInstanceHelper<FVoxelDataAssetInstance, UVoxelDataAsset>
{
public:
	const TVoxelSharedPtr<FVoxelDataAssetData> Data;
	const bool bSubtractiveAsset;
	const FIntVector PositionOffset;

public:
	FVoxelDataAssetInstance(
		const TVoxelSharedPtr<FVoxelDataAssetData>& Data,
		const bool bSubtractiveAsset,
		const FIntVector& PositionOffset)
		: Data(Data)
		, bSubtractiveAsset(bSubtractiveAsset)
		, PositionOffset(PositionOffset)
	{
	}

	//~ Begin FVoxelWorldGeneratorInstance Interface
	template<bool bCustomTransform>
	inline float GetValueImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		const FVector P = GetLocalPosition<bCustomTransform>(LocalToWorld, X, Y, Z);
		const float Value = Data->GetInterpolatedValue(P.X, P.Y, P.Z, bSubtractiveAsset ? FVoxelValue::Full() : FVoxelValue::Empty());
		if (Items.IsEmpty() || FVoxelValue(Value) == (bSubtractiveAsset ? FVoxelValue::Empty() : FVoxelValue::Full()))
		{
			// No need to merge as we are the best value possible
			return Value;
		}
		else
		{
			const auto NextStack = Items.GetNextStack(X, Y, Z);
			const auto NextValue = NextStack.Get<v_flt>(X, Y, Z, LOD);
			return FVoxelUtilities::MergeAsset<v_flt>(Value, NextValue, bSubtractiveAsset);
		}
	}
	
	template<bool bCustomTransform>
	inline FVoxelMaterial GetMaterialImpl(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		const FVector P = GetLocalPosition<bCustomTransform>(LocalToWorld, X, Y, Z);
		
		const auto GetDataMaterial = [&]()
		{
			if (Data->HasMaterials())
			{
				return Data->GetInterpolatedMaterial(P.X, P.Y, P.Z);
			}
			else
			{
				return FVoxelMaterial::Default();
			}
		};

		if (Items.IsEmpty())
		{
			return GetDataMaterial();
		}

		const FVoxelValue Value = FVoxelValue(Data->GetInterpolatedValue(P.X, P.Y, P.Z, bSubtractiveAsset ? FVoxelValue::Full() : FVoxelValue::Empty()));
		if ((bSubtractiveAsset && Value == FVoxelValue::Empty()) ||
			(!bSubtractiveAsset && Value == FVoxelValue::Full()))
		{
			// No need to check further down
			return GetDataMaterial();
		}

		const auto NextStack = Items.GetNextStack(X, Y, Z);
		const auto NextValue = NextStack.Get<FVoxelValue>(X, Y, Z, LOD);
		if (bSubtractiveAsset ? Value >= NextValue : Value <= NextValue)
		{
			// We have a better value
			return GetDataMaterial();
		}
		else
		{
			return NextStack.Get<FVoxelMaterial>(X, Y, Z, LOD);
		}
	}
	
	template<bool bCustomTransform>
	TVoxelRange<v_flt> GetValueRangeImpl(const FTransform& LocalToWorld, const FIntBox& WorldBounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		// Will be raised if world bounds are overriden for a data asset
		// Still useful to detect queries errors
		ensureVoxelSlow(!bCustomTransform || GetLocalBounds().ApplyTransform(LocalToWorld).Contains(WorldBounds));

		if (WorldBounds.Intersect(bCustomTransform ? GetLocalBounds().ApplyTransform(LocalToWorld) : GetLocalBounds()))
		{
			return { -1, 1 };
		}
		else if (Items.IsEmpty())
		{
			return bSubtractiveAsset ? -1 : 1;
		}
		else
		{
			const auto NextStack = Items.GetNextStack(WorldBounds);
			if (NextStack.IsValid())
			{
				return NextStack.GetValueRange(WorldBounds, LOD);
			}
			else
			{
				return TVoxelRange<v_flt>::Infinite();
			}
		}
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelWorldGeneratorInstance Interface

public:
	inline UVoxelDataAsset* GetOwner() const
	{
		return Data->Owner.Get();
	}

private:
	template<bool bCustomTransform>
	FORCEINLINE FVector GetLocalPosition(const FTransform& LocalToWorld, v_flt X, v_flt Y, v_flt Z) const
	{
		if (bCustomTransform)
		{
			const auto LocalPosition = LocalToWorld.InverseTransformPosition(FVector(X, Y, Z));
			X = LocalPosition.X;
			Y = LocalPosition.Y;
			Z = LocalPosition.Z;
		}
		X -= PositionOffset.X;
		Y -= PositionOffset.Y;
		Z -= PositionOffset.Z;
		return FVector(X, Y, Z);
	}
	FORCEINLINE FIntBox GetLocalBounds() const
	{
		return FIntBox(PositionOffset, PositionOffset + Data->GetSize());
	}
};

///////////////////////////////////////////////////////////////////////////////

void FVoxelDataAssetData::Serialize(FArchive& Ar, uint32 ValueConfigFlag, uint32 MaterialConfigFlag, int32 VoxelCustomVersion)
{
	FScopedSlowTask Serializing(2.f);
	
	Ar.UsingCustomVersion(FVoxelCustomVersion::GUID);
	Ar << Size;

	Serializing.EnterProgressFrame(1.f, LOCTEXT("SerializingValues", "Serializing values"));
	FVoxelSerializationUtilities::SerializeValues(Ar, Values, ValueConfigFlag, VoxelCustomVersion);

	Serializing.EnterProgressFrame(1.f, LOCTEXT("SerializingMaterials", "Serializing materials"));
	FVoxelSerializationUtilities::SerializeMaterials(Ar, Materials, MaterialConfigFlag, VoxelCustomVersion);

	if (Size.X * Size.Y * Size.Z != Values.Num() || (Materials.Num() > 0 && Size.X * Size.Y * Size.Z != Materials.Num()))
	{
		Ar.SetError();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelDataAsset::GetTransformableInstance()
{
	TryLoad();
	return MakeVoxelShared<FVoxelDataAssetInstance>(
		Data,
		bSubtractiveAsset,
		PositionOffset);
}

void UVoxelDataAsset::SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const
{
	auto& DataInstance = static_cast<const FVoxelDataAssetInstance&>(Instance);
	FString Path;
	auto* Owner = DataInstance.GetOwner();
	if (Owner)
	{
		Path = Owner->GetPathName();
	}
	else
	{
		FVoxelMessages::Error(LOCTEXT("InvalidOwner", "Invalid Data Asset Owner, saving an empty path"));
	}
	Ar << Path;
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelDataAsset::LoadInstance(FArchive& Ar) const
{
	FString Path;
	Ar << Path;

	if (auto* Asset = LoadObject<UVoxelDataAsset>(GetTransientPackage(), *Path))
	{
		return Asset->GetTransformableInstance();
	}
	else
	{
		Ar.SetError();
		FVoxelMessages::Error(FText::Format(LOCTEXT("InvalidPath", "Invalid Data Asset Path: '{0}'"), FText::FromString(Path)));
		return MakeVoxelShared<FVoxelTransformableEmptyWorldGeneratorInstance>();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<const FVoxelDataAssetData> UVoxelDataAsset::GetData()
{
	TryLoad();
	return Data.ToSharedRef();
}

TVoxelSharedRef<FVoxelDataAssetData> UVoxelDataAsset::MakeData()
{
	return MakeVoxelShared<FVoxelDataAssetData>(this);
}

void UVoxelDataAsset::SetData(const TVoxelSharedRef<FVoxelDataAssetData>& InData)
{
	ensure(InData->Owner == this);
	Data = InData;
	Save();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataAsset::Save()
{
	FScopedSlowTask Saving(2.f);

	Modify();

	VoxelCustomVersion = FVoxelCustomVersion::LatestVersion;
	MaterialConfigFlag = GVoxelMaterialConfigFlag;

	Saving.EnterProgressFrame(1.f, LOCTEXT("Serializing", "Serializing"));

	FBufferArchive Archive(true);
	Data->Serialize(Archive, ValueConfigFlag, MaterialConfigFlag, VoxelCustomVersion);

	Saving.EnterProgressFrame(1.f, LOCTEXT("Compressing", "Compressing"));
	FVoxelSerializationUtilities::CompressData(Archive, CompressedData);

	SyncProperties();
}


void UVoxelDataAsset::Load()
{
	if (CompressedData.Num() == 0)
	{
		// Nothing to load
		return;
	}
	
	TArray<uint8> UncompressedData;
	if (!FVoxelSerializationUtilities::DecompressData(CompressedData, UncompressedData))
	{
		FVoxelMessages::Error(LOCTEXT("VoxelDataAssetDecompressionFailed", "Decompression failed, data is corrupted"), this);
		return;
	}

	FMemoryReader Reader(UncompressedData);
	Data->Serialize(Reader, ValueConfigFlag, MaterialConfigFlag, VoxelCustomVersion);
	if (Reader.IsError())
	{
		FVoxelMessages::Error(LOCTEXT("VoxelDataAssetDecompressionReaderError", "Serialization failed, data is corrupted"), this);
	}

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
#undef LOCTEXT_NAMESPACE