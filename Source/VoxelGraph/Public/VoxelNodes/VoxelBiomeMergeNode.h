// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNode.h"
#include "VoxelNodeStructs.h"
#include "VoxelBiomeMergeNode.generated.h"

// Merge biomes by generating nodes to do so.
// Will also generate function separators, so you need to make all your data go through this
// (check the Additional Data field)
UCLASS(DisplayName = "Biome Merge", Category = "Biomes")
class VOXELGRAPH_API UVoxelNode_BiomeMerge : public UVoxelNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Biomes")
	TArray<FString> Biomes;

	// Additional data you want to send through the generated function separator
	UPROPERTY(EditAnywhere, Category = "Biomes")
	TArray<FVoxelNamedDataPin> AdditionalData;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Tolerance = 0.00001;

	UVoxelNode_BiomeMerge() = default;

	virtual int32 GetMinInputPins() const override;
	virtual int32 GetMaxInputPins() const override;
	virtual int32 GetOutputPinsCount() const override;

	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;

	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;

	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};