// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelPlaceableItemsNodes.generated.h"

class UVoxelGraphDataItemConfig;

UCLASS(DisplayName = "Data Item Sample", Category = "Placeable Items")
class VOXELGRAPH_API UVoxelNode_DataItemSample : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	
public:
	UVoxelNode_DataItemSample();
	
	// Only items matching the channels ticked here will be sampled (only the items matching (Mask & Item.Mask) != 0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config", meta = (Bitmask, BitmaskEnum = EVoxel32BitMask))
	int32 Mask = 1;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelDataItemCombineMode CombineMode = EVoxelDataItemCombineMode::Min;

};

UCLASS(DisplayName = "Data Item Parameters", Category = "Placeable Items")
class VOXELGRAPH_API UVoxelNode_DataItemParameters : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ReconstructNode, NonNull))
	UVoxelGraphDataItemConfig* Config;

	// If no parameters are provided these will be used
	UPROPERTY(EditAnywhere, Category = "Preview")
	TMap<FName, float> PreviewValues;

public:
	UVoxelNode_DataItemParameters() = default;

	//~ Begin UVoxelNode Interface
	virtual int32 GetOutputPinsCount() const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;
	EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;
	//~ End UVoxelNode Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

	TArray<v_flt> GetPreviewValues() const;
};