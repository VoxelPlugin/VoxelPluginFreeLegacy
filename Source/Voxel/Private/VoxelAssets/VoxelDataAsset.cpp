// Copyright 2020 Phyronnaz

#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelWorldGenerators/VoxelEmptyWorldGenerator.h"
#include "VoxelWorldGeneratorHelpers.h"
#include "VoxelWorldGeneratorInstance.inl"
#include "VoxelData/VoxelTransformableWorldGeneratorHelper.h"
#include "VoxelCustomVersion.h"
#include "VoxelMessages.h"
#include "VoxelSerializationUtilities.h"

#include "Engine/Texture2D.h"
#include "Misc/ScopedSlowTask.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelDataAssetMemory);

class FVoxelDataAssetInstance : public TVoxelWorldGeneratorInstanceHelper<FVoxelDataAssetInstance, UVoxelDataAsset>
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
	v_flt GetValueImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		X -= PositionOffset.X;
		Y -= PositionOffset.Y;
		Z -= PositionOffset.Z;
		
		return Data->GetInterpolatedValue(X, Y, Z, bSubtractiveAsset ? FVoxelValue::Full() : FVoxelValue::Empty());
	}
	
	FVoxelMaterial GetMaterialImpl(v_flt X, v_flt Y, v_flt Z, int32 LOD, const FVoxelItemStack& Items) const
	{
		X -= PositionOffset.X;
		Y -= PositionOffset.Y;
		Z -= PositionOffset.Z;
		
		if (Data->HasMaterials())
		{
			return Data->GetInterpolatedMaterial(X, Y, Z);
		}
		else
		{
			return FVoxelMaterial::Default();
		}
	}
	
	TVoxelRange<v_flt> GetValueRangeImpl(const FIntBox& Bounds, int32 LOD, const FVoxelItemStack& Items) const
	{
		if (Bounds.Intersect(GetLocalBounds()))
		{
			return { -1, 1 };
		}
		else
		{
			return bSubtractiveAsset ? -1 : 1;
		}
	}
	FVector GetUpVector(v_flt X, v_flt Y, v_flt Z) const override final
	{
		return FVector::UpVector;
	}
	//~ End FVoxelWorldGeneratorInstance Interface

public:
	UVoxelDataAsset* GetOwner() const
	{
		return Data->Owner.Get();
	}

private:
	FORCEINLINE FIntBox GetLocalBounds() const
	{
		return FIntBox(PositionOffset, PositionOffset + Data->GetSize());
	}
};

///////////////////////////////////////////////////////////////////////////////

void FVoxelDataAssetData::SetSize(const FIntVector& NewSize, bool bCreateMaterials)
{
	VOXEL_FUNCTION_COUNTER();
	check(int64(NewSize.X) * int64(NewSize.Y) * int64(NewSize.Z) < MAX_int32);

	// Somewhat thread safe
	Values.SetNumUninitialized(NewSize.X * NewSize.Y * NewSize.Z);
	Materials.SetNumUninitialized(bCreateMaterials ? NewSize.X * NewSize.Y * NewSize.Z : 0);
	Size = NewSize;

	ensure(Size.GetMin() > 0);
	ensure(Size.GetMax() > 1); // Else it'll be considered empty
	UpdateStats();
}

void FVoxelDataAssetData::Serialize(FArchive& Ar, uint32 ValueConfigFlag, uint32 MaterialConfigFlag, int32 VoxelCustomVersion)
{
	FVoxelScopedSlowTask Serializing(2.f);
	
	Ar.UsingCustomVersion(FVoxelCustomVersion::GUID);
	Ar << Size;

	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing values"));
	FVoxelSerializationUtilities::SerializeValues(Ar, Values, ValueConfigFlag, VoxelCustomVersion);

	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing materials"));
	FVoxelSerializationUtilities::SerializeMaterials(Ar, Materials, MaterialConfigFlag, VoxelCustomVersion);

	if (Size.X * Size.Y * Size.Z != Values.Num() || (Materials.Num() > 0 && Size.X * Size.Y * Size.Z != Materials.Num()))
	{
		Ar.SetError();
	}

	UpdateStats();
}

void FVoxelDataAssetData::UpdateStats() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataAssetMemory, AllocatedSize);
	AllocatedSize = Values.GetAllocatedSize() + Materials.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataAssetMemory, AllocatedSize);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelDataAsset::GetInstance()
{
	return GetInstanceImpl();
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelDataAsset::GetTransformableInstance()
{
	return MakeVoxelShared<TVoxelTransformableWorldGeneratorHelper<FVoxelDataAssetInstance>>(GetInstanceImpl(), bSubtractiveAsset);
}

void UVoxelDataAsset::SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const
{
	auto& DataInstance = *static_cast<const TVoxelTransformableWorldGeneratorHelper<FVoxelDataAssetInstance>&>(Instance).WorldGenerator;
	FString Path;
	auto* Owner = DataInstance.GetOwner();
	if (Owner)
	{
		Path = Owner->GetPathName();
	}
	else
	{
		FVoxelMessages::Error("Invalid Data Asset Owner, saving an empty path");
	}
	Ar << Path;
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelDataAsset::LoadInstance(FArchive& Ar) const
{
	FString Path;
	Ar << Path;

	if (auto* Asset = LoadObject<UVoxelDataAsset>(nullptr, *Path))
	{
		return Asset->GetTransformableInstance();
	}
	else
	{
		Ar.SetError();
		FVoxelMessages::Error("Invalid Data Asset Path: " + Path);
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

TVoxelSharedRef<FVoxelDataAssetInstance> UVoxelDataAsset::GetInstanceImpl()
{
	TryLoad();
	return MakeVoxelShared<FVoxelDataAssetInstance>(
		Data,
		bSubtractiveAsset,
		PositionOffset);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UVoxelDataAsset::Save()
{
	FVoxelScopedSlowTask Saving(2.f);

	Modify();

	VoxelCustomVersion = FVoxelCustomVersion::LatestVersion;
	ValueConfigFlag = GVoxelValueConfigFlag;
	MaterialConfigFlag = GVoxelMaterialConfigFlag;

	Saving.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing"));

	FBufferArchive Archive(true);
	Data->Serialize(Archive, ValueConfigFlag, MaterialConfigFlag, VoxelCustomVersion);

	Saving.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Compressing"));
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
		FVoxelMessages::Error("Decompression failed, data is corrupted", this);
		return;
	}

	FMemoryReader Reader(UncompressedData);
	Data->Serialize(Reader, ValueConfigFlag, MaterialConfigFlag, VoxelCustomVersion);
	if (Reader.IsError())
	{
		FVoxelMessages::Error("Serialization failed, data is corrupted", this);
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