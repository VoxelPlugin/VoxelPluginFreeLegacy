// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "Engine/EngineTypes.h"
#include "VoxelGenerators/VoxelGenerator.h"
#include "VoxelHeightmapAsset.generated.h"

class UTexture2D;

template<typename T>
struct TVoxelHeightmapAssetData;
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

/**
 * Asset that holds 2D information.
 */
UCLASS(Abstract, BlueprintType)
class VOXEL_API UVoxelHeightmapAsset : public UVoxelTransformableGeneratorWithBounds 
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generator Settings")
	bool bInfiniteExtent = false;

	// Additional thickness in voxels below the heightmap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generator Settings", meta = (EditCondition = "!bInfiniteExtent"))
	float AdditionalThickness = 0;

	// Higher precision can improve render quality, but voxel values are lower (hardness not constant)
	// Set this to the max delta height you can have between 2 adjacent pixels, in voxels
	// Need to be increased if the shadows/normals aren't nice, and decreased if the edit speed isn't coherent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generator Settings", meta = (ClampMin = 1))
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
	void TryLoad(TVoxelHeightmapAssetData<T>& Data);

	template<typename T>
	void SaveData(const TVoxelHeightmapAssetData<T>& Data);

	template<typename T>
	void LoadData(TVoxelHeightmapAssetData<T>& Data);

	template<typename T>
	void SyncProperties(const TVoxelHeightmapAssetData<T>& Data);

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
	
	//~ Begin UVoxelGenerator Interface
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
	virtual FVoxelIntBox GetBounds() const override;
	//~ End UVoxelGenerator Interface

private:
	TVoxelSharedPtr<TVoxelHeightmapAssetData<float>> Data;
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
	
	//~ Begin UVoxelGenerator Interface
	virtual TVoxelSharedRef<FVoxelGeneratorInstance> GetInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableGeneratorInstance> GetTransformableInstance() override;
	virtual FVoxelIntBox GetBounds() const override;
	//~ End UVoxelGenerator Interface

private:
	TVoxelSharedPtr<TVoxelHeightmapAssetData<uint16>> Data;
};