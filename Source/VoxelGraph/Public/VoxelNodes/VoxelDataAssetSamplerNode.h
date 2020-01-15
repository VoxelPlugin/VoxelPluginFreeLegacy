// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelExposedNodes.h"
#include "VoxelConfigEnums.h"
#include "VoxelDataAssetSamplerNode.generated.h"

class UVoxelDataAsset;

// Voxel data asset sampler
UCLASS(DisplayName = "Data Asset Sampler", Category = "Heightmap")
class VOXELGRAPH_API UVoxelNode_DataAssetSampler : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	UVoxelDataAsset* Asset;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	bool bBilinearInterpolation = true;
	
	UVoxelNode_DataAssetSampler();

	//~ Begin UVoxelNode Interface
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual FText GetTitle() const override;
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	//~ End UVoxelNode Interface

#if WITH_EDITOR
	//~ Begin UVoxelExposedNode Interface
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object) override;
	//~ End UVoxelExposedNode Interface
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};