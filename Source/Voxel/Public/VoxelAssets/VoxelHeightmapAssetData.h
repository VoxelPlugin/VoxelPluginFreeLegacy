// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelRange.h"

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Heightmap Assets Memory"), STAT_VoxelHeightmapAssetMemory, STATGROUP_VoxelMemory, VOXEL_API);

namespace FVoxelHeightmapAssetDataVersion
{
	enum Type : int32
	{
		BeforeCustomVersionWasAdded,
		SHARED_PlaceableItemsInSave,
		SHARED_AssetItemsImportValueMaterials,
		SHARED_DataAssetScale,
		SHARED_RemoveVoxelGrass,
		SHARED_DataAssetTransform,
		SHARED_RemoveEnableVoxelSpawnedActorsEnableVoxelGrass,
		SHARED_FoliagePaint,
		SHARED_ValueConfigFlagAndSaveGUIDs,
		SHARED_SingleValues,
		NoVoxelMaterialInHeightmapAssets,
		FixMissingMaterialsInHeightmapAssets,
		SHARED_AddUserFlagsToSaves,
		SHARED_StoreSpawnerMatricesRelativeToComponent,
		SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage,
		UseTArray64,
		SerializeHeightRangeMips,
		
		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};

template<typename T>
struct TVoxelHeightmapAssetData
{	
public:
	TVoxelHeightmapAssetData()
	{
		ClearData();
	}
	~TVoxelHeightmapAssetData()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelHeightmapAssetMemory, AllocatedSize);
	}

public:
	int64 GetWidth() const
	{
		return Width;
	}
	int64 GetHeight() const
	{
		return Height;
	}
	inline T GetMinHeight() const
	{
		return MinHeight;
	}
	inline T GetMaxHeight() const
	{
		return MaxHeight;
	}

public:
	void SetSize(int64 NewWidth, int64 NewHeight, bool bCreateMaterials, EVoxelMaterialConfig InMaterialConfig);
	void SetAllHeightsTo(T NewHeight);

	void ClearData()
	{
		// Make it safe to sample an empty asset
		SetSize(2, 2, false, {});
		SetAllHeightsTo(0);
	}
	
public:
	bool HasMaterials() const
	{
		return Materials.Num() > 0;
	}
	bool IsEmpty() const
	{
		return Heights.Num() <= 4 && Materials.Num() == 0;
	}
	int64 GetAllocatedSize() const
	{
		return AllocatedSize;
	}
	
public:
	int64 GetNumHeightRangeMips() const
	{
		return HeightRangeMips.Num();
	}

	const TVoxelRange<T>& GetHeightRange(int64 X, int64 Y, int64 Mip, EVoxelSamplerMode SamplerMode) const;
	// Max: excluded
	TVoxelRange<T> GetHeightRange(TVoxelRange<int64> X, TVoxelRange<int64> Y, EVoxelSamplerMode SamplerMode) const;

private:
	void GetHeightRangeLocalCoordinates(int64 Mip, int64 X, int64 Y, int64& LocalX, int64& LocalY) const;

	FORCEINLINE TVoxelRange<T>& GetHeightRangeLocal(int64 Mip, int64 LocalX, int64 LocalY)
	{
		return HeightRangeMips[Mip].Get(LocalX, LocalY);
	}
	FORCEINLINE const TVoxelRange<T>& GetHeightRangeLocal(int64 Mip, int64 LocalX, int64 LocalY) const
	{
		return HeightRangeMips[Mip].Get(LocalX, LocalY);
	}

	void FixHeightRangeLocalCoordinates(int64 Mip, int64& LocalX, int64& LocalY, EVoxelSamplerMode SamplerMode) const;

	void InitializeHeightRangeMips();

public:
	FORCEINLINE bool IsValidIndex(int64 X, int64 Y) const
	{
		return (0 <= X && X < Width) && (0 <= Y && Y < Height);
	}
	FORCEINLINE int64 GetIndex(int64 X, int64 Y) const
	{
		checkVoxelSlow(IsValidIndex(X, Y));
		return X + Width * Y;
	}

	void SetHeight(int64 X, int64 Y, T NewHeight);

	void SetMaterial_RGB(int64 X, int64 Y, FColor Color);
	void SetMaterial_SingleIndex(int64 X, int64 Y, uint8 SingleIndex);
	void SetMaterial_MultiIndex(int64 X, int64 Y, const FVoxelMaterial& Material);

	FORCEINLINE T GetHeightUnsafe(int64 X, int64 Y) const
	{
		return GetHeightUnsafe(GetIndex(X, Y));
	}
	FORCEINLINE FVoxelMaterial GetMaterialUnsafe(int64 X, int64 Y) const
	{
		return GetMaterialUnsafe(GetIndex(X, Y));
	}
	FORCEINLINE T GetHeightUnsafe(int64 Index) const
	{
		return Heights[Index];
	}
	FVoxelMaterial GetMaterialUnsafe(int64 Index) const;

public:
	void TileCoordinates(int64& X, int64& Y) const;
	void ClampCoordinates(int64& X, int64& Y) const;

	T GetHeight(int64 X, int64 Y, EVoxelSamplerMode Mode) const;
	FVoxelMaterial GetMaterial(int64 X, int64 Y, EVoxelSamplerMode Mode) const;
	
	T GetHeight(int32 X, int32 Y, EVoxelSamplerMode Mode) const
	{
		return GetHeight(int64(X), int64(Y), Mode);
	}
	FVoxelMaterial GetMaterial(int32 X, int32 Y, EVoxelSamplerMode Mode) const
	{
		return GetMaterial(int64(X), int64(Y), Mode);
	}

	float GetHeight(float X, float Y, EVoxelSamplerMode Mode) const;
	FVoxelMaterial GetMaterial(float X, float Y, EVoxelSamplerMode Mode) const;

public:
	const auto& GetRawHeights() const
	{
		return Heights;
	}

public:
	void Serialize(FArchive& Ar, uint32 MaterialConfigFlag, FVoxelHeightmapAssetDataVersion::Type Version, bool& bNeedToSave);
	
private:
	TNoGrowArray64<T> Heights;
	TNoGrowArray64<uint8> Materials;

	// In theory these fit in int32, but it's safer to use 64 bit math everywhere
	int64 Width = -1;
	int64 Height = -1;
	
	T MinHeight = 0;
	T MaxHeight = 0;

	EVoxelMaterialConfig MaterialConfig{};

	struct FHeightRangeMip
	{
		int64 Width = -1;
		int64 Height = -1;

		TArray<TVoxelRange<T>> Data;

		TVoxelRange<T>& Get(int64 X, int64 Y)
		{
			checkVoxelSlow(0 <= X && X < Width && 0 <= Y && Y < Height);
			return Data[X + Width * Y];
		}
		const TVoxelRange<T>& Get(int64 X, int64 Y) const
		{
			checkVoxelSlow(0 <= X && X < Width && 0 <= Y && Y < Height);
			return Data[X + Width * Y];
		}

		friend FArchive& operator<<(FArchive& Ar, FHeightRangeMip& Mip)
		{
			Ar << Mip.Width;
			Ar << Mip.Height;
			Ar << Mip.Data;
			return Ar;
		}
	};
	TArray<FHeightRangeMip, TInlineAllocator<16>> HeightRangeMips;
	
private:
	int64 AllocatedSize = 0;

	void UpdateStats();
};