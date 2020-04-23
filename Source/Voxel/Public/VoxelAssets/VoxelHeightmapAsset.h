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

template<typename>
class TVoxelHeightmapAssetInstance;

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Heightmap Assets Memory"), STAT_VoxelHeightmapAssetMemory, STATGROUP_VoxelMemory, VOXEL_API);

template<typename T>
struct TVoxelHeightmapAssetData
{	
	TWeakObjectPtr<UVoxelHeightmapAsset> const Owner;

	explicit TVoxelHeightmapAssetData(UVoxelHeightmapAsset* Owner)
		: Owner(Owner)
	{
	}
	~TVoxelHeightmapAssetData()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelHeightmapAssetMemory, AllocatedSize);
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
	inline void SetSize(int32 NewWidth, int32 NewHeight, bool bCreateMaterials, EVoxelMaterialConfig InMaterialConfig)
	{
		// Somewhat thread safe
		Heights.SetNumUninitialized(NewWidth * NewHeight);

		if (bCreateMaterials)
		{
			Materials.SetNumUninitialized(NewWidth * NewHeight * (
				InMaterialConfig == EVoxelMaterialConfig::RGB
				? 4
				: InMaterialConfig == EVoxelMaterialConfig::SingleIndex
				? 1
				: 3));
		}
		else
		{
			Materials.Empty();
		}
		
		Width = NewWidth;
		Height = NewHeight;
		MinHeight = TNumericLimits<T>::Max();
		MaxHeight = TNumericLimits<T>::Lowest();

		MaterialConfig = InMaterialConfig;

		UpdateStats();
	}
	inline void SetAllHeightsTo(T NewHeight)
	{
		for (auto& HeightIt : Heights)
		{
			HeightIt = NewHeight;
		}
		MinHeight = MaxHeight = NewHeight;
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
	inline void SetMaterial_RGB(int32 X, int32 Y, FColor Color)
	{
		SetMaterial_RGB(GetIndex(X, Y), Color);
	}
	inline void SetMaterial_SingleIndex(int32 X, int32 Y, uint8 SingleIndex)
	{
		SetMaterial_SingleIndex(GetIndex(X, Y), SingleIndex);
	}
	inline void SetMaterial_DoubleIndex(int32 X, int32 Y, uint8 IndexA, uint8 IndexB, uint8 Alpha)
	{
		SetMaterial_DoubleIndex(GetIndex(X, Y), IndexA, IndexB, Alpha);
	}
	
	inline void SetHeight(int32 Index, T NewHeight)
	{
		MaxHeight = FMath::Max(MaxHeight, NewHeight);
		MinHeight = FMath::Min(MinHeight, NewHeight);
		Heights[Index] = NewHeight;
	}
	inline void SetMaterial_RGB(int32 Index, FColor Color)
	{
		checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::RGB);
		Materials[4 * Index + 0] = Color.R;
		Materials[4 * Index + 1] = Color.G;
		Materials[4 * Index + 2] = Color.B;
		Materials[4 * Index + 3] = Color.A;
	}
	inline void SetMaterial_SingleIndex(int32 Index, uint8 SingleIndex)
	{
		checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::SingleIndex);
		Materials[Index] = SingleIndex;
	}
	inline void SetMaterial_DoubleIndex(int32 Index, uint8 IndexA, uint8 IndexB, uint8 Alpha)
	{
		checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::DoubleIndex);
		Materials[3 * Index + 0] = IndexA;
		Materials[3 * Index + 1] = IndexB;
		Materials[3 * Index + 2] = Alpha;
	}

	inline T GetHeightUnsafe(int32 X, int32 Y) const
	{
		return GetHeightUnsafe(GetIndex(X, Y));
	}
	inline FVoxelMaterial GetMaterialUnsafe(int32 X, int32 Y) const
	{
		return GetMaterialUnsafe(GetIndex(X, Y));
	}
	inline T GetHeightUnsafe(int32 Index) const
	{
		return Heights[Index];
	}
	inline FVoxelMaterial GetMaterialUnsafe(int32 Index) const
	{
		FVoxelMaterial Material(ForceInit);
		switch (MaterialConfig)
		{
		case EVoxelMaterialConfig::RGB:
			Material.SetR(Materials[4 * Index + 0]);
			Material.SetG(Materials[4 * Index + 1]);
			Material.SetB(Materials[4 * Index + 2]);
			Material.SetA(Materials[4 * Index + 3]);
			break;
		case EVoxelMaterialConfig::SingleIndex:
			Material.SetSingleIndex_Index(Materials[Index]);
			break;
		case EVoxelMaterialConfig::DoubleIndex:
		default:
			Material.SetDoubleIndex_IndexA(Materials[3 * Index + 0]);
			Material.SetDoubleIndex_IndexB(Materials[3 * Index + 1]);
			Material.SetDoubleIndex_Blend(Materials[3 * Index + 2]);
			break;
		}
		return Material;
	}

public:
	inline void TileCoordinates(int32& X, int32& Y) const
	{
		X = FVoxelUtilities::PositiveMod(X, GetWidth());
		Y = FVoxelUtilities::PositiveMod(Y, GetHeight());
	}
	inline void ClampCoordinates(int32& X, int32& Y) const
	{
		X = FMath::Clamp(X, 0, GetWidth() - 1);
		Y = FMath::Clamp(Y, 0, GetHeight() - 1);
	}
	inline T GetHeight(int32 X, int32 Y, EVoxelSamplerMode Mode, T DefaultHeight) const
	{
		if (!IsValidIndex(X, Y))
		{
			if (Mode == EVoxelSamplerMode::Tile)
			{
				TileCoordinates(X, Y);
			}
			else
			{
				ClampCoordinates(X, Y);
			}
			if (!ensureMsgf(IsValidIndex(X, Y), TEXT("X: %d, Y: %d"), X, Y))
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
			}
			else
			{
				ClampCoordinates(X, Y);
			}
			if (!ensureMsgf(IsValidIndex(X, Y), TEXT("X: %d, Y: %d"), X, Y))
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
	inline const TArray<T>& GetRawHeights() const
	{
		return Heights;
	}

public:
	void Serialize(FArchive& Ar, uint32 MaterialConfigFlag, int32 VoxelCustomVersion);
	
private:
	TArray<T> Heights = { 0, 0, 0, 0 };
	TArray<uint8> Materials;
	
	int32 Width = 2;
	int32 Height = 2;
	T MinHeight = 0;
	T MaxHeight = 0;

	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig::RGB;
	
private:
	uint32 AllocatedSize = 0;

	void UpdateStats()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelHeightmapAssetMemory, AllocatedSize);
		AllocatedSize = Heights.GetAllocatedSize() + Materials.GetAllocatedSize();
		INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelHeightmapAssetMemory, AllocatedSize);
	}
};

template<typename T>
struct TVoxelHeightmapAssetSamplerWrapper
{
	const float Scale;
	const float HeightScale;
	const float HeightOffset;
	const TVoxelSharedRef<TVoxelHeightmapAssetData<T>> Data;

	explicit TVoxelHeightmapAssetSamplerWrapper(UVoxelHeightmapAsset* Asset);

	inline float GetHeight(v_flt X, v_flt Y, EVoxelSamplerMode SamplerMode) const
	{
		return HeightOffset + HeightScale * Data->GetHeight(float(X / Scale), float(Y / Scale), SamplerMode, Data->GetMinHeight());
	}
	inline FVoxelMaterial GetMaterial(v_flt X, v_flt Y, EVoxelSamplerMode SamplerMode) const
	{
		return Data->GetMaterial(float(X / Scale), float(Y / Scale), SamplerMode);
	}

	inline void SetHeight(int32 X, int32 Y, float Height)
	{
		ensureVoxelSlowNoSideEffects(Scale == 1.f);
		Height -= HeightOffset;
		Height /= HeightScale;
		Height = FMath::Clamp<float>(Height, TNumericLimits<T>::Lowest(), TNumericLimits<T>::Max());
		if (TIsSame<T, float>::Value)
		{
			Data->SetHeight(X, Y, Height);
		}
		else
		{
			Data->SetHeight(X, Y, FMath::RoundToInt(Height));
		}
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
class VOXEL_API UVoxelHeightmapAsset : public UVoxelTransformableWorldGeneratorWithBounds 
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

	// If false, will have meshes on the sides. If true, will extend infinitely.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap World Generator Settings")
	bool bInfiniteExtent = false;

	// Additional thickness in voxels below the heightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap World Generator Settings", meta = (EditCondition = "!bInfiniteExtent"))
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

	template<typename T>
	FIntBox GetBoundsImpl() const;
	
protected:
	virtual void Serialize(FArchive& Ar) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Heightmap Info")
	int32 Width;
	UPROPERTY(VisibleAnywhere, Category = "Heightmap Info")
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
	TVoxelSharedRef<TVoxelHeightmapAssetData<float>> GetDataSharedPtr();
	void Save();

	TVoxelSharedRef<TVoxelHeightmapAssetInstance<float>> GetInstanceImpl();
	
	//~ Begin UVoxelWorldGenerator Interface
	virtual TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
	virtual FIntBox GetBounds() const override;
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
	TVoxelSharedRef<TVoxelHeightmapAssetData<uint16>> GetDataSharedPtr();
	void Save();

	TVoxelSharedRef<TVoxelHeightmapAssetInstance<uint16>> GetInstanceImpl();
	
	//~ Begin UVoxelWorldGenerator Interface
	virtual TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override;
	virtual FIntBox GetBounds() const override;
	//~ End UVoxelWorldGenerator Interface

private:
	TVoxelSharedRef<TVoxelHeightmapAssetData<uint16>> Data = MakeVoxelShared<TVoxelHeightmapAssetData<uint16>>(this);
};