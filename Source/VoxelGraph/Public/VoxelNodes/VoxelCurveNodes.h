// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelExposedNodes.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveLinearColor.h"
#include "VoxelCurveNodes.generated.h"

// Apply a float curve
UCLASS(DisplayName = "Float Curve", Category = "Curve")
class VOXELGRAPH_API UVoxelNode_Curve : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	GENERATED_EXPOSED_VOXELNODE_BODY(Curve)

public:
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (NonNull))
	UCurveFloat* Curve;

	UVoxelNode_Curve();

	virtual FText GetTitle() const override;
};

// Apply a color curve
// TODO option to output color
UCLASS(DisplayName = "Color Curve", Category = "Curve")
class VOXELGRAPH_API UVoxelNode_CurveColor : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	GENERATED_EXPOSED_VOXELNODE_BODY(Curve)

public:
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (NonNull))
	UCurveLinearColor* Curve;

	UVoxelNode_CurveColor();

	virtual FText GetTitle() const override;
};