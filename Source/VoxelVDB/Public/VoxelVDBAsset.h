// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelRange.h"
#include "VoxelStaticArray.h"
#include "VoxelWorldGenerators/VoxelWorldGenerator.h"
#include "VoxelVDBAsset.generated.h"

class UVoxelVDBAsset;
class FVoxelVDBAssetInstance;
struct FVoxelMaterial;
struct FVoxelVDBAssetDataChannel;

UENUM()
enum class EVoxelVDBChannel
{
	Density,
	R,
	G,
	B,
	A,
	U0,
	U1,
	U2,
	U3,
	V0,
	V1,
	V2,
	V3,
	Max UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FVoxelVDBImportChannelConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	EVoxelVDBChannel TargetChannel = EVoxelVDBChannel::Density;

	// If true, will automatically assign the Min/Max used for normalization to the min & max of the input data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	bool bAutoMinMax = false;

	// Min/Max, used to normalize the input data. Min = 0 and Max = 1 does nothing.
	// Result = (Value - Min) / (Max - Min)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (EditCondition = "!bNormalize"))
	float Min = 0.f;
	
	// Min/Max, used to normalize the input data. Min = 0 and Max = 1 does nothing.
	// Result = (Value - Min) / (Max - Min)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel", meta = (EditCondition = "!bNormalize"))
	float Max = 1.f;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class VOXELVDB_API FVoxelVDBAssetData
{
public:
	const TWeakObjectPtr<UVoxelVDBAsset> Owner;

	explicit FVoxelVDBAssetData(TWeakObjectPtr<UVoxelVDBAsset> Owner);
	~FVoxelVDBAssetData();

public:
	bool LoadVDB(const FString& Path, FString& OutError, const TFunction<bool(TMap<FName, FVoxelVDBImportChannelConfig>&)>& GetChannelConfigs);
	bool SaveVDB(const FString& Path, FString& OutError) const;
	
public:
	bool IsValid() const;

	void Save(TArray<uint8>& OutData) const;
	void Load(const TArray<uint8>& Data);

	FVoxelIntBoxWithValidity GetBounds() const;
	
public:
	float GetValue(double X, double Y, double Z) const;
	FVoxelMaterial GetMaterial(double X, double Y, double Z) const;

	TVoxelRange<float> GetValueRange(const FVoxelIntBox& Bounds) const;
	
private:
	TVoxelStaticArray<TUniquePtr<FVoxelVDBAssetDataChannel>, int32(EVoxelVDBChannel::Max)> Channels{ ForceInit };
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS(HideDropdown, BlueprintType)
class VOXELVDB_API UVoxelVDBAsset : public UVoxelTransformableWorldGeneratorWithBounds
{
	GENERATED_BODY()
		
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Asset")
	FVoxelIntBox Bounds;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Asset")
	float MemorySizeInMB;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Import")
	FString ImportPath;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Import")
	TMap<FName, FVoxelVDBImportChannelConfig> ChannelConfigs;
	
public:
	//~ Begin UVoxelWorldGenerator Interface
	virtual FVoxelIntBox GetBounds() const override;
	virtual TVoxelSharedRef<FVoxelWorldGeneratorInstance> GetInstance() override;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> GetTransformableInstance() override final;
	virtual void SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const override final;
	virtual TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> LoadInstance(FArchive& Ar) const override final;
	//~ End UVoxelWorldGenerator Interface

public:
	TVoxelSharedRef<const FVoxelVDBAssetData> GetData();
	TVoxelSharedRef<FVoxelVDBAssetData> MakeData();
	void SetData(const TVoxelSharedRef<FVoxelVDBAssetData>& InData);

protected:
	void Save();
	void Load();
	
	void TryLoad();

	TVoxelSharedRef<FVoxelVDBAssetInstance> GetInstanceImpl();

protected:
	virtual void Serialize(FArchive& Ar) override;

private:
	TVoxelSharedRef<FVoxelVDBAssetData> Data = MakeVoxelShared<FVoxelVDBAssetData>(this);

private:
	UPROPERTY()
	int32 VoxelCustomVersion;

	TArray<uint8> CompressedData;
};