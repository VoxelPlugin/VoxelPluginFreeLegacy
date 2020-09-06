// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"

class AVoxelWorld;
class UTexture2D;
class FVoxelDataAssetInstance;

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Data Assets Memory"), STAT_VoxelDataAssetMemory, STATGROUP_VoxelMemory, VOXEL_API);

namespace FVoxelDataAssetDataVersion
{
	enum Type : int32
	{
		BeforeCustomVersionWasAdded,
		SHARED_PlaceableItemsInSave,
		SHARED_AssetItemsImportValueMaterials,
		SHARED_DataAssetScale,
		SHARED_RemoveVoxelGrass,
		SHARED_DataAssetTransform,
		RemoveEnableVoxelSpawnedActorsEnableVoxelGrass,
		SHARED_FoliagePaint,
		ValueConfigFlagAndSaveGUIDs,
		SHARED_SingleValues,
		SHARED_NoVoxelMaterialInHeightmapAssets,
		SHARED_FixMissingMaterialsInHeightmapAssets,
		SHARED_AddUserFlagsToSaves,
		SHARED_StoreSpawnerMatricesRelativeToComponent,
		SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage,
		
		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};

struct VOXEL_API FVoxelDataAssetData
{
	FVoxelDataAssetData() = default;
	~FVoxelDataAssetData()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataAssetMemory, AllocatedSize);
	}

public:
	FORCEINLINE FIntVector GetSize() const
	{
		return Size;
	}

	void SetSize(const FIntVector& NewSize, bool bCreateMaterials);

	FORCEINLINE bool HasMaterials() const
	{
		return Materials.Num() > 0;
	}
	FORCEINLINE bool IsEmpty() const
	{
		return Values.Num() <= 1 && Materials.Num() <= 1;
	}
	
public:
	FORCEINLINE int32 GetIndex(int32 X, int32 Y, int32 Z) const
	{
		checkVoxelSlow(IsValidIndex(X, Y, Z));
		return X + Size.X * Y + Size.X * Size.Y * Z;
	}
	FORCEINLINE bool IsValidIndex(int32 X, int32 Y, int32 Z) const
	{
		return (0 <= X && X < Size.X) &&
				(0 <= Y && Y < Size.Y) &&
				(0 <= Z && Z < Size.Z);
	}
	FORCEINLINE bool IsValidIndex(float X, float Y, float Z) const
	{
		return (0 <= X && X <= Size.X - 1) &&
				(0 <= Y && Y <= Size.Y - 1) &&
				(0 <= Z && Z <= Size.Z - 1);
	}

	FORCEINLINE void SetValue(int32 X, int32 Y, int32 Z, const FVoxelValue& NewValue)
	{
		checkVoxelSlow(Values.IsValidIndex(GetIndex(X, Y, Z)));
		Values.GetData()[GetIndex(X, Y, Z)] = NewValue;
	}
	FORCEINLINE void SetMaterial(int32 X, int32 Y, int32 Z, const FVoxelMaterial& NewMaterial)
	{
		checkVoxelSlow(Materials.IsValidIndex(GetIndex(X, Y, Z)));
		Materials.GetData()[GetIndex(X, Y, Z)] = NewMaterial;
	}

	template<typename T>
	FORCEINLINE FVoxelValue GetValueUnsafe(T X, T Y, T Z) const
	{
		static_assert(TIsSame<T, int32>::Value, "should be int32");
		checkVoxelSlow(Values.IsValidIndex(GetIndex(X, Y, Z)));
		return Values.GetData()[GetIndex(X, Y, Z)];
	}
	template<typename T>
	FORCEINLINE FVoxelMaterial GetMaterialUnsafe(T X, T Y, T Z) const
	{
		static_assert(TIsSame<T, int32>::Value, "should be int32");
		checkVoxelSlow(Materials.IsValidIndex(GetIndex(X, Y, Z)));
		return Materials.GetData()[GetIndex(X, Y, Z)];
	}

public:
	FVoxelValue GetValue(int32 X, int32 Y, int32 Z, FVoxelValue DefaultValue) const;
	FVoxelMaterial GetMaterial(int32 X, int32 Y, int32 Z) const;

	template<typename T>
	FVoxelValue GetValue(T X, T Y, T Z, FVoxelValue DefaultValue) const = delete;
	template<typename T>
	FVoxelMaterial GetMaterial(T X, T Y, T Z) const = delete;

	float GetInterpolatedValue(float X, float Y, float Z, FVoxelValue DefaultValue, float Tolerance = 0.0001f) const;
	FVoxelMaterial GetInterpolatedMaterial(float X, float Y, float Z, float Tolerance = 0.0001f) const;

public:
	void Serialize(FArchive& Ar, uint32 ValueConfigFlag, uint32 MaterialConfigFlag, FVoxelDataAssetDataVersion::Type Version);

public:
	TNoGrowArray<FVoxelValue>& GetRawValues()
	{
		return Values;
	}
	TNoGrowArray<FVoxelMaterial>& GetRawMaterials()
	{
		return Materials;
	}
	const TNoGrowArray<FVoxelValue>& GetRawValues() const
	{
		return Values;
	}
	const TNoGrowArray<FVoxelMaterial>& GetRawMaterials() const
	{
		return Materials;
	}

public:
	int64 GetAllocatedSize() const
	{
		return AllocatedSize;
	}	

private:
	// Not 0 to avoid crashes if empty
	FIntVector Size = FIntVector(1, 1, 1);
	TNoGrowArray<FVoxelValue> Values = { FVoxelValue::Empty() };
	TNoGrowArray<FVoxelMaterial> Materials = { FVoxelMaterial::Default() };
	mutable int64 AllocatedSize = 0;

	void UpdateStats() const;
};