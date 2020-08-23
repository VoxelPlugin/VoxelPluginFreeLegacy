// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "VoxelDataAsset.generated.h"

class AVoxelWorld;
class UTexture2D;
class UVoxelDataAsset;
class FVoxelDataAssetInstance;
struct FVoxelDataAssetData;

#define DATA_ASSET_THUMBNAIL_RES 128

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
	virtual FVoxelIntBox GetBounds() const override
	{
		return FVoxelIntBox(PositionOffset, PositionOffset + Size);
	}

public:
	UVoxelDataAsset();

	//~ Begin UVoxelWorldGenerator Interface
	virtual TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override final;
	//~ End UVoxelWorldGenerator Interface

public:
	TVoxelSharedRef<const FVoxelDataAssetData> GetData();
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
	TVoxelSharedPtr<FVoxelDataAssetData> Data;

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