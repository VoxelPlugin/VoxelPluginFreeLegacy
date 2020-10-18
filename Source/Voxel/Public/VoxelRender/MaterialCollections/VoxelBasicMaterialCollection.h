// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterialCollectionBase.h"
#include "VoxelBasicMaterialCollection.generated.h"

USTRUCT(BlueprintType)
struct FVoxelBasicMaterialCollectionLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	uint8 LayerIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UMaterialInterface* LayerMaterial = nullptr;

	inline bool operator==(int32 Other) const
	{
		return LayerIndex == Other;
	}
};

// Material collection that does not generate any blending and is just a list of materials
UCLASS(BlueprintType)
class VOXEL_API UVoxelBasicMaterialCollection : public UVoxelMaterialCollectionBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
	TArray<FVoxelBasicMaterialCollectionLayer> Layers;
	
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual int32 GetMaxMaterialIndices() const override;
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const override;
	virtual UMaterialInterface* GetIndexMaterial(uint8 Index) const override;
	virtual TArray<FMaterialInfo> GetMaterials() const override;
	virtual int32 GetMaterialIndex(FName Name) const override;
	//~ End UVoxelMaterialCollectionBase Interface

#if WITH_EDITOR
	// Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// End UObject Interface
#endif
};