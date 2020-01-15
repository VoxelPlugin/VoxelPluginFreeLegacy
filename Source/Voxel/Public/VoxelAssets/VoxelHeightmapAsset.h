// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "VoxelMaterial.h"
#include "VoxelMathUtilities.h"
#include "VoxelConfigEnums.h"
#include "VoxelWorldGeneratorHelpers.h"
#include "VoxelHeightmapAsset.generated.h"

class UVoxelHeightmapAsset;
class UTexture2D;

DECLARE_MEMORY_STAT_EXTERN(TEXT("Voxel Heightmap Assets Memory"), STAT_VoxelHeightmapAssetMemory, STATGROUP_VoxelMemory, VOXEL_API);

template<typename T>
struct TVoxelHeightmapAssetData
{	
	TWeakObjectPtr<UVoxelHeightmapAsset> const Owner;
	
	TVoxelHeightmapAssetData(UVoxelHeightmapAsset* Owner)
		: Owner(Owner)
	{
		INC_DWORD_STAT_BY(STAT_VoxelHeightmapAssetMemory, GetAllocatedSize());
	}
	~TVoxelHeightmapAssetData()
	{
		DEC_DWORD_STAT_BY(STAT_VoxelHeightmapAssetMemory, GetAllocatedSize());
	}

public:
	inline int32 GetWidth() const
	{
		return Width;
	}
	inline int32 GetHeight() const
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
	inline void SetSize(int32 NewWidth, int32 NewHeight, bool bCreateMaterials)
	{
		DEC_DWORD_STAT_BY(STAT_VoxelHeightmapAssetMemory, GetAllocatedSize());

		// Somewhat thread safe
		Heights.SetNumUninitialized(NewWidth * NewHeight);
		Materials.SetNumUninitialized(bCreateMaterials ? NewWidth * NewHeight : 0);
		Width = NewWidth;
		Height = NewHeight;
		MinHeight = TNumericLimits<T>::Max();
		MaxHeight = TNumericLimits<T>::Min();

		INC_DWORD_STAT_BY(STAT_VoxelHeightmapAssetMemory, GetAllocatedSize());
	}
	
	inline bool HasMaterials() const
	{
		return Materials.Num() > 0;
	}
	inline bool IsEmpty() const
	{
		return Heights.Num() <= 4 && Materials.Num() == 0;
	}
	inline int32 GetAllocatedSize() const
	{
		return Heights.GetAllocatedSize() + Materials.GetAllocatedSize();
	}

public:
	inline int32 GetIndex(int32 X, int32 Y) const
	{
		return X + Width * Y;
	}
	inline bool IsValidIndex(int32 X, int32 Y) const
	{
		return (0 <= X && X < Width) && (0 <= Y && Y < Height);
	}

	inline void SetHeight(int32 X, int32 Y, T NewHeight)
	{
		SetHeight(GetIndex(X, Y), NewHeight);
	}
	inline void SetMaterial(int32 X, int32 Y, const FVoxelMaterial& NewMaterial)
	{
		Materials[GetIndex(X, Y)] = NewMaterial;
	}
	inline void SetHeight(int32 Index, T NewHeight)
	{
		MaxHeight = FMath::Max(MaxHeight, NewHeight);
		MinHeight = FMath::Min(MinHeight, NewHeight);
		Heights[Index] = NewHeight;
	}
	inline void SetMaterial(int32 Index, const FVoxelMaterial& NewMaterial)
	{
		Materials[Index] = NewMaterial;
	}

	inline T GetHeightUnsafe(int32 X, int32 Y) const
	{
		return Heights[GetIndex(X, Y)];
	}
	inline FVoxelMaterial GetMaterialUnsafe(int32 X, int32 Y) const
	{
		return Materials[GetIndex(X, Y)];
	}
	inline T GetHeightUnsafe(int32 Index) const
	{
		return Heights[Index];
	}
	inline FVoxelMaterial GetMaterialUnsafe(int32 Index) const
	{
		return Materials[Index];
	}

public:
	inline void TileCoordinates(int32& X, int32& Y) const
	{
		X = FVoxelUtilities::PositiveMod(X, GetWidth());
		Y = FVoxelUtilities::PositiveMod(Y, GetHeight());
	}
	inline T GetHeight(int32 X, int32 Y, EVoxelSamplerMode Mode, T DefaultHeight) const
	{
		if (!IsValidIndex(X, Y))
		{
			if (Mode == EVoxelSamplerMode::Tile)
			{
				TileCoordinates(X, Y);
				if (!ensureMsgf(IsValidIndex(X, Y), TEXT("X: %d, Y: %d"), X, Y))
				{
					return DefaultHeight;
				}
			}
			else
			{
				return DefaultHeight;
			}
		}
		return GetHeightUnsafe(X, Y);
	}
	inline FVoxelMaterial GetMaterial(int32 X, int32 Y, EVoxelSamplerMode Mode) const
	{
		if (!IsValidIndex(X, Y))
		{
			if (Mode == EVoxelSamplerMode::Tile)
			{
				TileCoordinates(X, Y);
				if (!ensureMsgf(IsValidIndex(X, Y), TEXT("X: %d, Y: %d"), X, Y))
				{
					return FVoxelMaterial::Default();
				}
			}
			else
			{
				return FVoxelMaterial::Default();
			}
		}
		return GetMaterialUnsafe(X, Y);
	}
	
	inline float GetHeight(float X, float Y, EVoxelSamplerMode Mode, T DefaultHeight) const
	{
		const int32 MinX = FMath::FloorToInt(X);
		const int32 MinY = FMath::FloorToInt(Y);
		
		const int32 MaxX = FMath::CeilToInt(X);
		const int32 MaxY = FMath::CeilToInt(Y);

		const float AlphaX = X - MinX;
		const float AlphaY = Y - MinY;

		return FVoxelUtilities::BilinearInterpolation<float>(
			GetHeight(MinX, MinY, Mode, DefaultHeight),
			GetHeight(MaxX, MinY, Mode, DefaultHeight),
			GetHeight(MinX, MaxY, Mode, DefaultHeight),
			GetHeight(MaxX, MaxY, Mode, DefaultHeight),
			AlphaX,
			AlphaY);
	}
	inline FVoxelMaterial GetMaterial(float X, float Y, EVoxelSamplerMode Mode) const
	{
		if (HasMaterials())
		{
			return GetMaterial(FMath::RoundToInt(X), FMath::RoundToInt(Y), Mode);
		}
		else
		{
			return FVoxelMaterial::Default();
		}
	}

public:
	void SerializeAsset(FArchive& Ar, uint32 MaterialConfigFlag, int32 VoxelCustomVersion);
	
private:
	TArray<T> Heights = { 0, 0, 0, 0 };
	TArray<FVoxelMaterial> Materials;
	int32 Width = 2;
	int32 Height = 2;
	T MinHeight = 0;
	T MaxHeight = 0;
};

template<typename T>
struct VOXEL_API TVoxelHeightmapAssetSamplerWrapper
{
	const float Scale;
	const float HeightScale;
	const float HeightOffset;
	const TVoxelSharedRef<const TVoxelHeightmapAssetData<T>> Data;

	TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset);

	inline float GetHeight(v_flt X, v_flt Y, EVoxelSamplerMode SamplerMode) const
	{
		return HeightOffset + HeightScale * Data->GetHeight(float(X / Scale), float(Y / Scale), SamplerMode, Data->GetMinHeight());
	}
	inline FVoxelMaterial GetMaterial(v_flt X, v_flt Y, EVoxelSamplerMode SamplerMode) const
	{
		return Data->GetMaterial(float(X / Scale), float(Y / Scale), SamplerMode);
	}
	inline float GetMinHeight() const
	{
		return HeightOffset + HeightScale * Data->GetMinHeight();
	}
	inline float GetMaxHeight() const
	{
		return HeightOffset + HeightScale * Data->GetMaxHeight();
	}
	inline float GetWidth() const
	{
		return Scale * Data->GetWidth();
	}
	inline float GetHeight() const
	{
		return Scale * Data->GetHeight();
	}
};

/**
 * Asset that holds 2D information.
 */
UCLASS(Abstract, BlueprintType)
class VOXEL_API UVoxelHeightmapAsset : public UVoxelWorldGenerator 
{
	GENERATED_BODY()

public:
	// XY Scale of the heightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Asset Settings", meta = (ClampMin = 0, DisplayName = "XY Scale"))
	float Scale = 1;

	// Height multiplier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Asset Settings", DisplayName = "Z Scale")
	float HeightScale = 1;

	// In voxels, applied after Z Scale
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Asset Settings")
	float HeightOffset = 0;

	// Additional thickness in voxels below the heightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap World Generator Settings")
	float AdditionalThickness = 0;

	// Higher precision can improve render quality, but voxel values are lower (hardness not constant)
	// Set this to the max delta height you can have between 2 adjacent pixels, in voxels
	// Need to be increased if the shadows/normals aren't nice, and decreased if the edit speed isn't coherent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap World Generator Settings", meta = (ClampMin = 1))
	float Precision = 4;

	UFUNCTION(BlueprintCallable, Category = "Voxel|Heightmap Asset")
	int32 GetWidth() const
	{
		return Width;
	}
	UFUNCTION(BlueprintCallable, Category = "Voxel|Heightmap Asset")
	int32 GetHeight() const
	{
		return Height;
	}

protected:
	template<typename T>
	void TryLoad(TVoxelSharedRef<TVoxelHeightmapAssetData<T>>& Data);

	template<typename T>
	void SaveData(const TVoxelSharedRef<TVoxelHeightmapAssetData<T>>& Data);

	template<typename T>
	void LoadData(TVoxelSharedRef<TVoxelHeightmapAssetData<T>>& Data);

	template<typename T>
	void SyncProperties(const TVoxelSharedRef<TVoxelHeightmapAssetData<T>>& Data);
	
protected:
	virtual void Serialize(FArchive& Ar) override;

private:
	UPROPERTY()
	int32 Width;
	UPROPERTY()
	int32 Height;
	
	UPROPERTY()
	int32 VoxelCustomVersion;
	
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

#if WITH_EDITOR
protected:
	template<typename T, typename U>
	UTexture2D* GetThumbnailInternal();

public:
	UTexture2D* GetThumbnail();
#endif
};

UCLASS(HideDropdown)
class VOXEL_API UVoxelHeightmapAssetFloat : public UVoxelHeightmapAsset
{
	GENERATED_BODY()

public:
	UVoxelHeightmapAssetFloat();

	TVoxelHeightmapAssetData<float>& GetData();
	TVoxelSharedRef<const TVoxelHeightmapAssetData<float>> GetDataSharedPtr();
	void Save();
	
	//~ Begin UVoxelWorldGenerator Interface
	TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override;
	//~ End UVoxelWorldGenerator Interface

private:
	TVoxelSharedRef<TVoxelHeightmapAssetData<float>> Data = MakeVoxelShared<TVoxelHeightmapAssetData<float>>(this);
};

USTRUCT()
struct FVoxelHeightmapImporterWeightmapInfos
{
	GENERATED_BODY()

	// The weightmap
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FFilePath File;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelRGBA Layer = EVoxelRGBA::R;
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	uint8 Index = 0;
};

UCLASS(HideDropdown)
class VOXEL_API UVoxelHeightmapAssetUINT16 : public UVoxelHeightmapAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Import configuration")
	FString Heightmap;

	UPROPERTY(VisibleAnywhere, Category = "Import configuration")
	EVoxelMaterialConfig MaterialConfig;

	UPROPERTY(VisibleAnywhere, Category = "Import configuration")
	TArray<FString> Weightmaps;

	UPROPERTY()
	TArray<FVoxelHeightmapImporterWeightmapInfos> WeightmapsInfos;

public:
	UVoxelHeightmapAssetUINT16();
	
	TVoxelHeightmapAssetData<uint16>& GetData();
	TVoxelSharedRef<const TVoxelHeightmapAssetData<uint16>> GetDataSharedPtr();
	void Save();

	//~ Begin UVoxelWorldGenerator Interface
	TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override;
	//~ End UVoxelWorldGenerator Interface

private:
	TVoxelSharedRef<TVoxelHeightmapAssetData<uint16>> Data = MakeVoxelShared<TVoxelHeightmapAssetData<uint16>>(this);
};