// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelExposedNodes.h"
#include "VoxelAssetPickerNode.generated.h"

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelAssetPickerNode : public UVoxelExposedNode
{
	GENERATED_BODY()
public:
	//~ Begin UVoxelAssetPickerNode Interface
	virtual UObject* GetAsset() const { return nullptr; }
	virtual UClass* GetAssetClass() const { return nullptr; }
	virtual void SetAsset(UObject* Object) {}
	virtual bool ShouldFilterAsset(const struct FAssetData& Asset) const { return false; }
	//~ End UVoxelAssetPickerNode Interface
};