// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterialCollectionBase.h"
#include "VoxelLandscapeMaterialCollection.generated.h"

struct FMaterialParameterInfo;
class UMaterialInstanceConstant;

USTRUCT(BlueprintType)
struct FVoxelLandscapeMaterialCollectionLayer
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	uint8 Index = 0;
};

USTRUCT(BlueprintType)
struct FVoxelLandscapeMaterialCollectionPermutation
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Config")
	FName Names[6];

	bool operator==(const FVoxelLandscapeMaterialCollectionPermutation& Other) const
	{
		return
			Names[0] == Other.Names[0] &&
			Names[1] == Other.Names[1] &&
			Names[2] == Other.Names[2] &&
			Names[3] == Other.Names[3] &&
			Names[4] == Other.Names[4] &&
			Names[5] == Other.Names[5];
	}

	FString ToString() const
	{
		return FString::Printf(TEXT("%s,%s,%s,%s,%s,%s"),
			*Names[0].ToString(),
			*Names[1].ToString(),
			*Names[2].ToString(),
			*Names[3].ToString(),
			*Names[4].ToString(),
			*Names[5].ToString());
	}
};

inline uint32 GetTypeHash(const FVoxelLandscapeMaterialCollectionPermutation& Key)
{
	return
		HashCombine(GetTypeHash(Key.Names[0]),
			HashCombine(GetTypeHash(Key.Names[1]),
				HashCombine(GetTypeHash(Key.Names[2]),
					HashCombine(GetTypeHash(Key.Names[3]),
						HashCombine(GetTypeHash(Key.Names[4]), GetTypeHash(Key.Names[5]))))));
}

// Material collection that does not generate any blending and is just a list of materials
UCLASS(BlueprintType)
class VOXEL_API UVoxelLandscapeMaterialCollection : public UVoxelMaterialCollectionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = 1, ClampMax = 6, UIMin = 1, UIMax = 6))
	int32 MaxMaterialsToBlendAtOnce = 6;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UMaterialInterface* Material = nullptr;

	// Add elements to this to reduce permutations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
	TMap<FName, bool> LayersToIgnore;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
	TArray<FVoxelLandscapeMaterialCollectionLayer> Layers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cache")
	mutable TMap<FVoxelLandscapeMaterialCollectionPermutation, UMaterialInstanceConstant*> MaterialCache;

public:
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual int32 GetMaxMaterialIndices() const override { return FMath::ClampAngle(MaxMaterialsToBlendAtOnce, 1, 6); }
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const override;
	virtual UMaterialInterface* GetIndexMaterial(uint8 Index) const override;
	virtual TArray<FMaterialInfo> GetMaterials() const override;
	virtual int32 GetMaterialIndex(FName Name) const override;
	virtual void InitializeCollection() override;
	//~ End UVoxelMaterialCollectionBase Interface

	// Begin UObject Interface
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	// End UObject Interface
	
#if WITH_EDITOR
	void BuildAllPermutations();
#endif

private:
	FVoxelLandscapeMaterialCollectionPermutation MakePermutation(const FVoxelMaterialIndices& Indices) const;
	UMaterialInstanceConstant* FindOrAddPermutation(const FVoxelLandscapeMaterialCollectionPermutation& Permutation) const;
	
#if WITH_EDITOR
	UMaterialInstanceConstant* CreateInstanceForPermutation(const FVoxelLandscapeMaterialCollectionPermutation& Permutation) const;
	void ForeachMaterialParameter(TFunctionRef<void(const FMaterialParameterInfo&, const FGuid&)> Lambda) const;
	bool NeedsToBeConvertedToVoxel() const;
	void FixupLayers();
	void CleanupCache() const;
#endif

	UPROPERTY(Transient)
	TMap<int32, FVoxelLandscapeMaterialCollectionLayer> IndicesToLayers;
};