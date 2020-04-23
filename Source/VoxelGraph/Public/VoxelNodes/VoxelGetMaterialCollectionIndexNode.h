// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelAssetPickerNode.h"
#include "VoxelGetMaterialCollectionIndexNode.generated.h"

class UMaterialInterface;

// Retrieve the index of a material function or a material instance in the voxel world material collection
UCLASS(DisplayName = "Get Material Collection Index", Category = "Material")
class VOXELGRAPH_API UVoxelNode_GetMaterialCollectionIndex : public UVoxelAssetPickerNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	UMaterialInterface* Material;

	UVoxelNode_GetMaterialCollectionIndex();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelAssetPickerNode Interface
	virtual UObject* GetAsset() const override;
	virtual UClass* GetAssetClass() const override;
	virtual void SetAsset(UObject* Object) override;
	virtual bool ShouldFilterAsset(const FAssetData& Asset) const override;
	//~ End UVoxelAssetPickerNode Interface
};