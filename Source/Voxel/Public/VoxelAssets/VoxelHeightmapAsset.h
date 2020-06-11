// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "VoxelRange.h"
#include "VoxelMaterial.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelConfigEnums.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorHelpers.h"
#include "VoxelHeightmapAsset.generated.h"

class UVoxelHeightmapAsset;
class UTexture2D;

template<typename>
class TVoxelHeightmapAssetInstance;

UENUM()
enum class EVoxelHeightmapImporterMaterialConfig : uint8
{
	RGB,
	FourWayBlend,
	FiveWayBlend,
	SingleIndex,
	MultiIndex
};

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Heightmap Assets Memory"), STAT_VoxelHeightmapAssetMemory, STATGROUP_VoxelMemory, VOXEL_API);

// TODO move to its own file
template<typename T>
struct TVoxelHeightmapAssetData
{	
public:
	TWeakObjectPtr<UVoxelHeightmapAsset> const Owner;

	explicit TVoxelHeightmapAssetData(UVoxelHeightmapAsset* Owner)
		: Owner(Owner)
	{
		// Make it safe to sample an empty asset
		SetSize(2, 2, false, {});
		SetAllHeightsTo(0);
	}
	~TVoxelHeightmapAssetData()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelHeightmapAssetMemory, AllocatedSize);
	}

public:
	int32 GetWidth() const
	{
		return Width;
	}
	int32 GetHeight() const
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
	void SetSize(int32 NewWidth, int32 NewHeight, bool bCreateMaterials, EVoxelMaterialConfig InMaterialConfig)
	{
		VOXEL_FUNCTION_COUNTER();
		
		check(NewWidth > 0 && NewHeight > 0);
		
		Heights.SetNumUninitialized(NewWidth * NewHeight);

		if (bCreateMaterials)
		{
			Materials.SetNumUninitialized(NewWidth * NewHeight * (
				InMaterialConfig == EVoxelMaterialConfig::RGB
				? 4
				: InMaterialConfig == EVoxelMaterialConfig::SingleIndex
				? 1
				: 7));
		}
		else
		{
			Materials.Empty();
		}
		
		Width = NewWidth;
		Height = NewHeight;
	
		MinHeight = PositiveInfinity<T>();
		MaxHeight = NegativeInfinity<T>();
		
		MaterialConfig = InMaterialConfig;

		UpdateStats();
		InitializeHeightRangeMips();
	}
	void SetAllHeightsTo(T NewHeight)
	{
		VOXEL_ASYNC_FUNCTION_COUNTER();
		
		for (auto& HeightIt : Heights)
		{
			HeightIt = NewHeight;
		}
		for (auto& HeightRangeMip : HeightRangeMips)
		{
			for (auto& HeightRange : HeightRangeMip.Data)
			{
				HeightRange = NewHeight;
			}
		}
	}
	
	bool HasMaterials() const
	{
		return Materials.Num() > 0;
	}
	bool IsEmpty() const
	{
		return Heights.Num() <= 4 && Materials.Num() == 0;
	}
	int32 GetAllocatedSize() const
	{
		return Heights.GetAllocatedSize() + Materials.GetAllocatedSize();
	}
	
public:
	int32 GetNumHeightRangeMips() const
	{
		return HeightRangeMips.Num();
	}
	const TVoxelRange<T>& GetHeightRange(int32 X, int32 Y, int32 Mip, EVoxelSamplerMode SamplerMode) const
	{
		int32 LocalX;
		int32 LocalY;
		GetHeightRangeLocalCoordinates(Mip, X, Y, LocalX, LocalY);

		FixHeightRangeLocalCoordinates(Mip, LocalX, LocalY, SamplerMode);
		
		return GetHeightRangeLocal(Mip, LocalX, LocalY);
	}
	// Max: excluded
	TVoxelRange<T> GetHeightRange(TVoxelRange<int32> X, TVoxelRange<int32> Y, EVoxelSamplerMode SamplerMode) const
	{
		if (!ensure(X.Min < X.Max && Y.Min < Y.Max))
		{
			return { MinHeight, MaxHeight };
		}

		int32 MinX = X.Min;
		int32 MaxX = X.Max;

		int32 MinY = Y.Min;
		int32 MaxY = Y.Max;

		if (SamplerMode == EVoxelSamplerMode::Clamp)
		{
			// Clamp min and max
			
			MinX = FMath::Clamp(MinX, 0, GetWidth() - 1);
			MinY = FMath::Clamp(MinY, 0, GetHeight() - 1);
			
			MaxX = FMath::Clamp(MaxX, 1, GetWidth());
			MaxY = FMath::Clamp(MaxY, 1, GetHeight());
		}
		else
		{
			// Tile min, and set max to be max one tile after
			// The coordinates will be tiled again when sampling the mips
			
			const int32 SizeX = MaxX - MinX;
			const int32 SizeY = MaxY - MinY;
			
			TileCoordinates(MinX, MinY);

			MaxX = MinX + SizeX;
			MaxY = MinY + SizeY;
		}
		
		const int32 SizeX = MaxX - MinX;
		const int32 SizeY = MaxY - MinY;

		const int32 MaxSize = FMath::Max(SizeX, SizeY);

		int32 Mip = FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(MaxSize);
		Mip = FMath::Clamp(Mip, 0, GetNumHeightRangeMips() - 1);

		const int32 MipPixelSize = RENDER_CHUNK_SIZE << Mip;
		
		const int32 LocalMinX = FVoxelUtilities::DivideFloor(MinX, MipPixelSize);
		const int32 LocalMinY = FVoxelUtilities::DivideFloor(MinY, MipPixelSize);

		// Note: since MaxX/Y are excluded, LocalMaxX/Y are too
		const int32 LocalMaxX = FVoxelUtilities::DivideCeil(MaxX, MipPixelSize);
		const int32 LocalMaxY = FVoxelUtilities::DivideCeil(MaxY, MipPixelSize);

		const int32 LocalSizeX = LocalMaxX - LocalMinX;
		const int32 LocalSizeY = LocalMaxY - LocalMinY;

		checkVoxelSlow(0 < LocalSizeX);
		checkVoxelSlow(0 < LocalSizeY);
		
		ensureVoxelSlow(LocalSizeX <= 2);
		ensureVoxelSlow(LocalSizeY <= 2);

		// Combine the range of all the overlapping pixels
		TOptional<TVoxelRange<T>> Range;
		for (int32 LocalX = LocalMinX; LocalX < LocalMaxX; LocalX++)
		{
			for (int32 LocalY = LocalMinY; LocalY < LocalMaxY; LocalY++)
			{
				int32 FixedLocalX = LocalX;
				int32 FixedLocalY = LocalY;

				// Tile the coordinates if needed
				FixHeightRangeLocalCoordinates(Mip, FixedLocalX, FixedLocalY, SamplerMode);
				ensureVoxelSlow(SamplerMode == EVoxelSamplerMode::Tile || (LocalX == FixedLocalX && LocalY == FixedLocalY));
				
				const auto LocalRange = GetHeightRangeLocal(Mip, FixedLocalX, FixedLocalY);

				Range = Range.IsSet() ? TVoxelRange<T>::Union(Range.GetValue(), LocalRange) : LocalRange;
			}
		}

		// Crashed
		ensureMsgf(
			Range.IsSet(),
			TEXT("LocalMinX: %d; LocalMaxX: %d; LocalMinY: %d; LocalMaxY: %d; Width: %d; Height: %d; MinX: %d; MaxX: %d; MinY: %d; MaxY: %d"),
			LocalMinX, LocalMaxX, LocalMinY, LocalMaxY, Width, Height, MinX, MaxX, MinY, MaxY);
		return Range.Get({ MinHeight, MaxHeight });
	}

private:
	FORCEINLINE void GetHeightRangeLocalCoordinates(int32 Mip, int32 X, int32 Y, int32& LocalX, int32& LocalY) const
	{
		checkVoxelSlow(IsValidIndex(X, Y));
		checkVoxelSlow(HeightRangeMips.IsValidIndex(Mip));

		constexpr int32 BaseMipDepth = FVoxelUtilities::IntLog2(RENDER_CHUNK_SIZE);
		const int32 MipDepth = BaseMipDepth + Mip;

		// Find the mip coordinates by dividing and flooring
		LocalX = X >> MipDepth;
		LocalY = Y >> MipDepth;
	}
	FORCEINLINE TVoxelRange<T>& GetHeightRangeLocal(int32 Mip, int32 LocalX, int32 LocalY)
	{
		return HeightRangeMips[Mip].Get(LocalX, LocalY);
	}
	FORCEINLINE const TVoxelRange<T>& GetHeightRangeLocal(int32 Mip, int32 LocalX, int32 LocalY) const
	{
		return HeightRangeMips[Mip].Get(LocalX, LocalY);
	}

	void FixHeightRangeLocalCoordinates(int32 Mip, int32& LocalX, int32& LocalY, EVoxelSamplerMode SamplerMode) const
	{
		auto& HeightRangeMip = HeightRangeMips[Mip];
		
		if (SamplerMode == EVoxelSamplerMode::Clamp)
		{
			LocalX = FMath::Clamp(LocalX, 0, HeightRangeMip.Width - 1);
			LocalY = FMath::Clamp(LocalY, 0, HeightRangeMip.Height - 1);
		}
		else
		{
			LocalX = FVoxelUtilities::PositiveMod(LocalX, HeightRangeMip.Width);
			LocalY = FVoxelUtilities::PositiveMod(LocalY, HeightRangeMip.Height);
		}
	}

	void InitializeHeightRangeMips()
	{
		const int32 NumHeightRangeMips = 1 + FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(FMath::Max(Width, Height));
		check(NumHeightRangeMips >= 1);

		HeightRangeMips.Empty();
		for (int32 MipIndex = 0; MipIndex < NumHeightRangeMips; MipIndex++)
		{
			const int32 MipPixelSize = RENDER_CHUNK_SIZE << MipIndex;

			TVoxelRange<T> Range;
			Range.Min = PositiveInfinity<T>();
			Range.Max = NegativeInfinity<T>();
			
			FHeightRangeMip NewMip;
			NewMip.Width = FVoxelUtilities::DivideCeil(Width, MipPixelSize);
			NewMip.Height = FVoxelUtilities::DivideCeil(Height, MipPixelSize);
			NewMip.Data.Init(Range, NewMip.Width * NewMip.Height);

			HeightRangeMips.Add(NewMip);
		}
	}
	
public:
	FORCEINLINE bool IsValidIndex(int32 X, int32 Y) const
	{
		return (0 <= X && X < Width) && (0 <= Y && Y < Height);
	}
	FORCEINLINE int32 GetIndex(int32 X, int32 Y) const
	{
		checkVoxelSlow(IsValidIndex(X, Y));
		return X + Width * Y;
	}

	void SetHeight(int32 X, int32 Y, T NewHeight)
	{
		Heights[GetIndex(X, Y)] = NewHeight;
		
		MaxHeight = FMath::Max(MaxHeight, NewHeight);
		MinHeight = FMath::Min(MinHeight, NewHeight);
		
		for (int32 Mip = 0; Mip < GetNumHeightRangeMips(); Mip++)
		{
			int32 LocalX;
			int32 LocalY;
			GetHeightRangeLocalCoordinates(Mip, X, Y, LocalX, LocalY);

			auto& Range = GetHeightRangeLocal(Mip, LocalX, LocalY);
			Range.Min = FMath::Min(Range.Min, NewHeight);
			Range.Max = FMath::Max(Range.Max, NewHeight);
		}
	}
	void SetMaterial_RGB(int32 X, int32 Y, FColor Color)
	{
		checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::RGB);
		const int32 Index = GetIndex(X, Y);
		
		Materials[4 * Index + 0] = Color.R;
		Materials[4 * Index + 1] = Color.G;
		Materials[4 * Index + 2] = Color.B;
		Materials[4 * Index + 3] = Color.A;
	}
	void SetMaterial_SingleIndex(int32 X, int32 Y, uint8 SingleIndex)
	{
		checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::SingleIndex);
		Materials[GetIndex(X, Y)] = SingleIndex;
	}
	void SetMaterial_MultiIndex(int32 X, int32 Y, const FVoxelMaterial& Material)
	{
		checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::MultiIndex);
		const int32 Index = GetIndex(X, Y);
		
		Materials[7 * Index + 0] = Material.GetMultiIndex_Blend0();
		Materials[7 * Index + 1] = Material.GetMultiIndex_Blend1();
		Materials[7 * Index + 2] = Material.GetMultiIndex_Blend2();
		Materials[7 * Index + 3] = Material.GetMultiIndex_Index0();
		Materials[7 * Index + 4] = Material.GetMultiIndex_Index1();
		Materials[7 * Index + 5] = Material.GetMultiIndex_Index2();
		Materials[7 * Index + 6] = Material.GetMultiIndex_Index3();
	}

	FORCEINLINE T GetHeightUnsafe(int32 X, int32 Y) const
	{
		return GetHeightUnsafe(GetIndex(X, Y));
	}
	FORCEINLINE FVoxelMaterial GetMaterialUnsafe(int32 X, int32 Y) const
	{
		return GetMaterialUnsafe(GetIndex(X, Y));
	}
	FORCEINLINE T GetHeightUnsafe(int32 Index) const
	{
		return Heights[Index];
	}
	FORCEINLINE FVoxelMaterial GetMaterialUnsafe(int32 Index) const
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
			Material.SetSingleIndex(Materials[Index]);
			break;
		case EVoxelMaterialConfig::MultiIndex:
		default:
			Material.SetMultiIndex_Blend0(Materials[7 * Index + 0]);
			Material.SetMultiIndex_Blend1(Materials[7 * Index + 1]);
			Material.SetMultiIndex_Blend2(Materials[7 * Index + 2]);
			Material.SetMultiIndex_Index0(Materials[7 * Index + 3]);
			Material.SetMultiIndex_Index1(Materials[7 * Index + 4]);
			Material.SetMultiIndex_Index2(Materials[7 * Index + 5]);
			Material.SetMultiIndex_Index3(Materials[7 * Index + 6]);
			break;
		}
		return Material;
	}

public:
	FORCEINLINE void TileCoordinates(int32& X, int32& Y) const
	{
		X = FVoxelUtilities::PositiveMod(X, GetWidth());
		Y = FVoxelUtilities::PositiveMod(Y, GetHeight());
	}
	FORCEINLINE void ClampCoordinates(int32& X, int32& Y) const
	{
		X = FMath::Clamp(X, 0, GetWidth() - 1);
		Y = FMath::Clamp(Y, 0, GetHeight() - 1);
	}
	T GetHeight(int32 X, int32 Y, EVoxelSamplerMode Mode) const
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
			checkVoxelSlow(IsValidIndex(X, Y));
		}
		return GetHeightUnsafe(X, Y);
	}
	FVoxelMaterial GetMaterial(int32 X, int32 Y, EVoxelSamplerMode Mode) const
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
			checkVoxelSlow(IsValidIndex(X, Y));
		}
		return GetMaterialUnsafe(X, Y);
	}
	
	float GetHeight(float X, float Y, EVoxelSamplerMode Mode) const
	{
		const int32 MinX = FMath::FloorToInt(X);
		const int32 MinY = FMath::FloorToInt(Y);
		
		const int32 MaxX = FMath::CeilToInt(X);
		const int32 MaxY = FMath::CeilToInt(Y);

		const float AlphaX = X - MinX;
		const float AlphaY = Y - MinY;

		return FVoxelUtilities::BilinearInterpolation<float>(
			GetHeight(MinX, MinY, Mode),
			GetHeight(MaxX, MinY, Mode),
			GetHeight(MinX, MaxY, Mode),
			GetHeight(MaxX, MaxY, Mode),
			AlphaX,
			AlphaY);
	}
	FVoxelMaterial GetMaterial(float X, float Y, EVoxelSamplerMode Mode) const
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
	const TArray<T>& GetRawHeights() const
	{
		return Heights;
	}

public:
	void Serialize(FArchive& Ar, uint32 MaterialConfigFlag, int32 VoxelCustomVersion);
	
private:
	TArray<T> Heights;
	TArray<uint8> Materials;
	
	int32 Width = -1;
	int32 Height = -1;
	
	T MinHeight = 0;
	T MaxHeight = 0;

	EVoxelMaterialConfig MaterialConfig{};

	struct FHeightRangeMip
	{
		int32 Width = -1;
		int32 Height = -1;

		TArray<TVoxelRange<T>> Data;

		TVoxelRange<T>& Get(int32 X, int32 Y)
		{
			checkVoxelSlow(0 <= X && X < Width && 0 <= Y && Y < Height);
			return Data[X + Width * Y];
		}
		const TVoxelRange<T>& Get(int32 X, int32 Y) const
		{
			checkVoxelSlow(0 <= X && X < Width && 0 <= Y && Y < Height);
			return Data[X + Width * Y];
		}
	};
	TArray<FHeightRangeMip, TInlineAllocator<16>> HeightRangeMips;
	
private:
	uint32 AllocatedSize = 0;

	void UpdateStats()
	{
		DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelHeightmapAssetMemory, AllocatedSize);
		AllocatedSize = Heights.GetAllocatedSize() + Materials.GetAllocatedSize() + HeightRangeMips.GetAllocatedSize();
		for (auto& Mip : HeightRangeMips)
		{
			AllocatedSize += Mip.Data.GetAllocatedSize();
		}
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

	float GetHeight(v_flt X, v_flt Y, EVoxelSamplerMode SamplerMode) const
	{
		return HeightOffset + HeightScale * Data->GetHeight(float(X / Scale), float(Y / Scale), SamplerMode);
	}
	FVoxelMaterial GetMaterial(v_flt X, v_flt Y, EVoxelSamplerMode SamplerMode) const
	{
		return Data->GetMaterial(float(X / Scale), float(Y / Scale), SamplerMode);
	}

	TVoxelRange<float> GetHeightRange(TVoxelRange<v_flt> X, TVoxelRange<v_flt> Y, EVoxelSamplerMode SamplerMode) const
	{
		return HeightOffset + HeightScale * TVoxelRange<float>(Data->GetHeightRange(
			{ FMath::FloorToInt(X.Min / Scale), FMath::CeilToInt(X.Max / Scale) }, 
			{ FMath::FloorToInt(Y.Min / Scale), FMath::CeilToInt(Y.Max / Scale) }, 
			SamplerMode));
	}

	void SetHeight(int32 X, int32 Y, float Height)
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

	float GetMinHeight() const
	{
		return HeightOffset + HeightScale * Data->GetMinHeight();
	}
	float GetMaxHeight() const
	{
		return HeightOffset + HeightScale * Data->GetMaxHeight();
	}
	
	float GetWidth() const
	{
		return Scale * Data->GetWidth();
	}
	float GetHeight() const
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
	FVoxelIntBox GetBoundsImpl() const;
	
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
	virtual FVoxelIntBox GetBounds() const override;
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
	EVoxelHeightmapImporterMaterialConfig MaterialConfig;

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
	virtual FVoxelIntBox GetBounds() const override;
	//~ End UVoxelWorldGenerator Interface

private:
	TVoxelSharedRef<TVoxelHeightmapAssetData<uint16>> Data = MakeVoxelShared<TVoxelHeightmapAssetData<uint16>>(this);
};