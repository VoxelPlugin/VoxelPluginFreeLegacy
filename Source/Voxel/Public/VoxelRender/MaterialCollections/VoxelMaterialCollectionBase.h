// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterialCollectionBase.generated.h"

struct FVoxelMaterialIndices;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelMaterialCollectionMaterialInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	uint8 Index = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TWeakObjectPtr<UMaterialInterface> Material;

	// If empty, will use the Material name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FName NameOverride;

	FName GetName() const;
};

UCLASS(Abstract)
class VOXEL_API UVoxelMaterialCollectionBase : public UObject
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelMaterialCollectionBase Interface
	// Called before the material collection is used (can be at runtime when dynamic renderer settings change)
	virtual void InitializeCollection() {}
	
	// Max number of material indices this collection can handle
	// eg if = 2, this collection can only blend between 2 indices at a time
	virtual int32 GetMaxMaterialIndices() const
	{
		unimplemented();
		return 0;
	}
	virtual bool EnableCubicFaces() const
	{
		return false;
	}
	virtual UMaterialInterface* GetVoxelMaterial(const FVoxelMaterialIndices& Indices, uint64 UniqueIdForErrors) const
	{
		unimplemented();
		return nullptr;
	}

	// Used by paint material customization. Some materials might be null.
	UFUNCTION(BlueprintPure, Category = "Voxel|Material Collection")
	virtual TArray<FVoxelMaterialCollectionMaterialInfo> GetMaterials() const { return {}; }
	
	UFUNCTION(BlueprintPure, Category = "Voxel|Material Collection")
	UMaterialInterface* GetIndexMaterial(uint8 Index) const;
	
	// Get the material index from a material or a layer name
	UFUNCTION(BlueprintPure, Category = "Voxel|Material Collection")
	int32 GetMaterialIndex(FName Name) const;
	//~ End UVoxelMaterialCollectionBase Interface
};