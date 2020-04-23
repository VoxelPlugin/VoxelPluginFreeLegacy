// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelMathUtilities.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelDataAsset.generated.h"

class FVoxelDataAssetInstance;
class UVoxelDataAsset;
class UTexture2D;

#define DATA_ASSET_THUMBNAIL_RES 128

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Data Assets Memory"), STAT_VoxelDataAssetMemory, STATGROUP_VoxelMemory, VOXEL_API);

struct VOXEL_API FVoxelDataAssetData
{
	TWeakObjectPtr<UVoxelDataAsset> const Owner;
	
	explicit FVoxelDataAssetData(UVoxelDataAsset* Owner)
		: Owner(Owner)
	{
	}
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
	FORCEINLINE FVoxelValue GetValue(int32 X, int32 Y, int32 Z, FVoxelValue DefaultValue) const
	{
		if (IsValidIndex(X, Y, Z))
		{
			return GetValueUnsafe(X, Y, Z);
		}
		else
		{
			return DefaultValue;
		}
	}
	FORCEINLINE FVoxelMaterial GetMaterial(int32 X, int32 Y, int32 Z) const
	{
		if (IsValidIndex(X, Y, Z))
		{
			return GetMaterialUnsafe(X, Y, Z);
		}
		else
		{
			return FVoxelMaterial::Default();
		}
	}
	template<typename T>
	FVoxelValue GetValue(T X, T Y, T Z, FVoxelValue DefaultValue) const = delete;
	template<typename T>
	FVoxelMaterial GetMaterial(T X, T Y, T Z) const = delete;

	inline float GetInterpolatedValue(float X, float Y, float Z, FVoxelValue DefaultValue) const
	{
		const int32 MinX = FMath::FloorToInt(X);
		const int32 MinY = FMath::FloorToInt(Y);
		const int32 MinZ = FMath::FloorToInt(Z);

		const int32 MaxX = FMath::CeilToInt(X);
		const int32 MaxY = FMath::CeilToInt(Y);
		const int32 MaxZ = FMath::CeilToInt(Z);

		const float AlphaX = X - MinX;
		const float AlphaY = Y - MinY;
		const float AlphaZ = Z - MinZ;

		return FVoxelUtilities::TrilinearInterpolation<float>(
			GetValue(MinX, MinY, MinZ, DefaultValue).ToFloat(),
			GetValue(MaxX, MinY, MinZ, DefaultValue).ToFloat(),
			GetValue(MinX, MaxY, MinZ, DefaultValue).ToFloat(),
			GetValue(MaxX, MaxY, MinZ, DefaultValue).ToFloat(),
			GetValue(MinX, MinY, MaxZ, DefaultValue).ToFloat(),
			GetValue(MaxX, MinY, MaxZ, DefaultValue).ToFloat(),
			GetValue(MinX, MaxY, MaxZ, DefaultValue).ToFloat(),
			GetValue(MaxX, MaxY, MaxZ, DefaultValue).ToFloat(),
			AlphaX,
			AlphaY,
			AlphaZ);
	}
	
	inline FVoxelMaterial GetInterpolatedMaterial(float X, float Y, float Z) const
	{
		// Note: might get better results by interpolating the material colors/UVs

		X = FMath::Clamp<float>(X, 0, Size.X - 1);
		Y = FMath::Clamp<float>(Y, 0, Size.Y - 1);
		Z = FMath::Clamp<float>(Z, 0, Size.Z - 1);
		
		auto* RESTRICT const ValuesPtr = Values.GetData();
		auto* RESTRICT const MaterialsPtr = Materials.GetData();
		const int32 MinX = FMath::FloorToInt(X);
		const int32 MinY = FMath::FloorToInt(Y);
		const int32 MinZ = FMath::FloorToInt(Z);
		const int32 MaxX = FMath::CeilToInt(X);
		const int32 MaxY = FMath::CeilToInt(Y);
		const int32 MaxZ = FMath::CeilToInt(Z);
		for (int32 ItX = MinX; ItX <= MaxX; ItX++)
		{
			for (int32 ItY = MinY; ItY <= MaxY; ItY++)
			{
				for (int32 ItZ = MinZ; ItZ <= MaxZ; ItZ++)
				{
					checkVoxelSlow(IsValidIndex(ItX, ItY, ItZ));
					const int32 Index = GetIndex(ItX, ItY, ItZ);
					checkVoxelSlow(Values.IsValidIndex(Index));
					checkVoxelSlow(Materials.IsValidIndex(Index));
					if (ValuesPtr[Index].IsEmpty()) continue;
					return MaterialsPtr[Index];
				}
			}
		}
		return MaterialsPtr[GetIndex(MinX, MinY, MinZ)];
	}

public:
	void Serialize(FArchive& Ar, uint32 ValueConfigFlag, uint32 MaterialConfigFlag, int32 VoxelCustomVersion);

public:
	inline TArray<FVoxelValue>& GetRawValues()
	{
		return Values;
	}
	inline TArray<FVoxelMaterial>& GetRawMaterials()
	{
		return Materials;
	}
	inline const TArray<FVoxelValue>& GetRawValues() const
	{
		return Values;
	}
	inline const TArray<FVoxelMaterial>& GetRawMaterials() const
	{
		return Materials;
	}

private:
	// Not 0 to avoid crashes if empty
	FIntVector Size = FIntVector(1, 1, 1);
	TArray<FVoxelValue> Values = { FVoxelValue::Empty() };
	TArray<FVoxelMaterial> Materials = { FVoxelMaterial::Default() };
	mutable uint32 AllocatedSize = 0;

	void UpdateStats() const;
};

UENUM()
enum class EVoxelDataAssetImportSource
{
	None,
	MagicaVox,
	RawVox,
	Mesh
};

/**
 * A Data Asset stores the values of every voxel inside it
 */
UCLASS(HideDropdown, BlueprintType)
class VOXEL_API UVoxelDataAsset : public UVoxelTransformableWorldGeneratorWithBounds
{
	GENERATED_BODY()

public:
	// If true, asset can be used to make holes in the world
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Config")
	bool bSubtractiveAsset = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Config")
	FIntVector PositionOffset;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Config")
	FIntVector Size;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Config")
	float UncompressedSizeInMB = 0;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Config")
	float CompressedSizeInMB = 0;

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Import")
	EVoxelDataAssetImportSource Source;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Import")
	TArray<FString> Paths;

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Data Asset")
	FIntVector GetSize() const
	{
		return Size;
	}

	UFUNCTION(BlueprintCallable, Category = "Voxel|Data Asset")
	virtual FIntBox GetBounds() const override
	{
		return FIntBox(PositionOffset, PositionOffset + Size);
	}

	//~ Begin UVoxelWorldGenerator Interface
	virtual TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override final;
	virtual void SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const override final;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> LoadInstance(FArchive& Ar) const override final;
	//~ End UVoxelWorldGenerator Interface

public:
	TVoxelSharedRef<const FVoxelDataAssetData> GetData();
	TVoxelSharedRef<FVoxelDataAssetData> MakeData();
	void SetData(const TVoxelSharedRef<FVoxelDataAssetData>& InData);

	TVoxelSharedRef<FVoxelDataAssetInstance> GetInstanceImpl();

protected:
	void Save();
	void Load();

	void TryLoad();
	void SyncProperties();

protected:
	virtual void Serialize(FArchive& Ar) override;

private:
	TVoxelSharedPtr<FVoxelDataAssetData> Data = MakeVoxelShared<FVoxelDataAssetData>(this);

private:
	UPROPERTY()
	int32 VoxelCustomVersion;
	
	UPROPERTY()
	uint32 ValueConfigFlag;
	
	UPROPERTY()
	uint32 MaterialConfigFlag;

	TArray<uint8> CompressedData;

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(NonTransactional)
	TArray<FColor> ThumbnailSave;

	UPROPERTY(Transient)
	UTexture2D* ThumbnailTexture;
#endif

public:
#if WITH_EDITOR
	void SetThumbnail(TArray<FColor>&& Colors);
	UTexture2D* GetThumbnail();
#endif

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	AVoxelWorld* VoxelWorldTemplate;
#endif
};