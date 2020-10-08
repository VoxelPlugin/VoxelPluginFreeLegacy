// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelExposedNodes.h"
#include "VoxelBiomeMapNode.generated.h"

class UTexture2D;

USTRUCT()
struct VOXELGRAPH_API FBiomeMapElement
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FColor Color;
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FString Name;
};

// Find the strength of biomes from a biome map. Note: Alpha is ignored when computing the color distance
UCLASS(DisplayName = "Biome Map Sampler", Category = "Biomes")
class VOXELGRAPH_API UVoxelNode_BiomeMapSampler : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Biomes")
	UTexture2D* Texture;

	// Distance = Max(Abs(ColorA - ColorB)). Values with a distance below or equal to this will be set to 1, value strictly above to 0
	UPROPERTY(EditAnywhere, Category = "Biomes", meta = (ClampMin = 0, ClampMax = 255, UIMin = 0, UIMax = 255))
	int Threshold = 0;

	UPROPERTY(EditAnywhere, Category = "Biomes", meta = (ReconstructNode))
	TArray<FBiomeMapElement> Biomes;

	TArray<FColor> GetColors() const;

	UVoxelNode_BiomeMapSampler();

	virtual int32 GetOutputPinsCount() const override;
	virtual FName GetOutputPinName(int32 PinIndex) const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;
	virtual FText GetTitle() const override;

	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Texture); }
};