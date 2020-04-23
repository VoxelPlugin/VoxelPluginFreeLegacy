// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelCachedMaterialCollection.h"
#include "VoxelInstancedMaterialCollection.generated.h"

class UMaterialInstance;

USTRUCT(BlueprintType)
struct FVoxelInstancedMaterialCollectionTemplates
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* Template1x = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* Template2x = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* Template3x = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* Template4x = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* Template5x = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInterface* Template6x = nullptr;
};

USTRUCT(BlueprintType)
struct FVoxelInstancedMaterialCollectionLayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	uint8 LayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	UMaterialInstance* LayerMaterialInstance = nullptr;

	inline bool operator==(int32 Other) const
	{
		return LayerIndex == Other;
	}
};

UCLASS(BlueprintType)
class VOXEL_API UVoxelInstancedMaterialCollection : public UVoxelCachedMaterialCollection
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = 1, ClampMax = 6, UIMin = 1, UIMax = 6))
	int32 MaxMaterialsToBlendAtOnce = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableTessellation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates")
	FVoxelInstancedMaterialCollectionTemplates NormalTemplates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Templates", meta = (EditCondition = "bEnableTessellation"))
	FVoxelInstancedMaterialCollectionTemplates TessellatedTemplates;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layers")
	TArray<FVoxelInstancedMaterialCollectionLayer> Layers;
	
public:
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual int32 GetMaxMaterialIndices() const override;
	virtual int32 GetMaterialIndex(FName Name) const override;
	//~ End UVoxelMaterialCollectionBase Interface

	//~ Begin UVoxelCachedMaterialCollection Interface
	virtual UMaterialInterface* GetVoxelMaterial_NotCached(const FVoxelMaterialIndices& Indices, bool bTessellation, uint64 UniqueIdForErrors) const override;
	//~ End UVoxelCachedMaterialCollection Interface

#if WITH_EDITOR
	// Begin UObject Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	// End UObject Interface
#endif
};