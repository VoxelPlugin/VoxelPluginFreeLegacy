// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelRender/MaterialCollections/VoxelCachedMaterialCollection.h"
#include "VoxelBlueprintMaterialCollection.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew)
class VOXEL_API UVoxelBlueprintMaterialCollectionInterface : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (ClampMin = 1, ClampMax = 6, UIMin = 1, UIMax = 6))
	int32 MaxMaterialsToBlendAtOnce = 6;

	// If false, faces will be merged
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bEnableCubicFaces = true;
	
	UFUNCTION(BlueprintImplementableEvent)
	void InitializeCollection();
	
	UFUNCTION(BlueprintImplementableEvent)
	UMaterialInterface* GetMaterialForIndex(int32 Index, EVoxelCubicFace Face);

	// Indices will have between 2 and MaxMaterialsToBlendAtOnce elements, and will be sorted
	UFUNCTION(BlueprintImplementableEvent)
	UMaterialInterface* GetMaterialForIndices(const TArray<uint8>& Indices);

	// This is used by the tools dropdown to select the material to paint
	UFUNCTION(BlueprintImplementableEvent)
	TArray<FVoxelMaterialCollectionMaterialInfo> GetMaterials();
};

// Material collection that does not generate any blending and is just a list of materials
UCLASS(BlueprintType)
class VOXEL_API UVoxelBlueprintMaterialCollection : public UVoxelCachedMaterialCollection
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", Instanced)
	UVoxelBlueprintMaterialCollectionInterface* Instance;
	
	//~ Begin UVoxelMaterialCollectionBase Interface
	virtual void InitializeCollection() override;
	virtual bool EnableCubicFaces() const override;
	virtual int32 GetMaxMaterialIndices() const override;
	virtual UMaterialInterface* GetVoxelMaterial_NotCached(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const override;
	virtual TArray<FVoxelMaterialCollectionMaterialInfo> GetMaterials() const override;
	//~ End UVoxelMaterialCollectionBase Interface
};