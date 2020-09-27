// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNoiseNodes.h"
#include "VoxelNoiseNodesMacros.h"
#include "VoxelGavoronoiNoiseNode.generated.h"

// 2D Gavoronoi Noise
// This noise can be directed, and is used to fake erosion
// See https://www.shadertoy.com/view/llsGWl
UCLASS(DisplayName = "2D Gavoronoi Noise", Category = "Noise|Gavoronoi Noise")
class VOXELGRAPH_API UVoxelNode_2DGavoronoiNoise : public UVoxelNode_NoiseNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_2DGavoronoiNoise();
	
	UPROPERTY(EditAnywhere, Category = "Gavoronoi Noise settings", meta = (UIMin = 0, UIMax = 0.5))
	float Jitter = 0.45;
		
	//~ Begin UVoxelNode_NoiseNode Interface
	virtual uint32 GetDimension() const override { return 2; }
	//~ End UVoxelNode_NoiseNode Interface
};

// 2D Gavoronoi Noise Fractal
// This noise can be directed, and is used to fake erosion
// See https://www.shadertoy.com/view/llsGWl
UCLASS(DisplayName = "2D Gavoronoi Noise Fractal", Category = "Noise|Gavoronoi Noise")
class VOXELGRAPH_API UVoxelNode_2DGavoronoiNoiseFractal : public UVoxelNode_NoiseNodeFractal
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_2DGavoronoiNoiseFractal();
	
	UPROPERTY(EditAnywhere, Category = "Gavoronoi Noise settings", meta = (UIMin = 0, UIMax = 0.5))
	float Jitter = 0.45;
		
	//~ Begin UVoxelNode_NoiseNode Interface
	virtual uint32 GetDimension() const override { return 2; }
	//~ End UVoxelNode_NoiseNode Interface
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// 2D Erosion
// Add this to your noise to fake erosion
// From https://www.shadertoy.com/view/MtGcWh
UCLASS(DisplayName = "2D Erosion", Category = "Noise|Erosion")
class VOXELGRAPH_API UVoxelNode_2DErosion : public UVoxelNode_NoiseNodeWithDerivativeFractal
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_2DErosion();

public:
	// Controls the jitter of the noise used for the "ravines"
	UPROPERTY(EditAnywhere, Category = "Erosion settings", meta = (UIMin = 0, UIMax = 0.5))
	float Jitter = 0.25;

public:
	//~ Begin UVoxelNode_NoiseNode Interface
	virtual uint32 GetDimension() const override { return 2; }
	//~ End UVoxelNode_NoiseNode Interface

	//~ Begin UObject Interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UObject Interface
};