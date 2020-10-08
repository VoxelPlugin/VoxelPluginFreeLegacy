// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelExposedNodes.h"
#include "VoxelCurveNodes.generated.h"

class UCurveFloat;
class UCurveLinearColor;

// Apply a float curve
UCLASS(DisplayName = "Float Curve", Category = "Curve")
class VOXELGRAPH_API UVoxelNode_Curve : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (NonNull))
	UCurveFloat* Curve;

	UVoxelNode_Curve();

	virtual FText GetTitle() const override;
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Curve); }
};

// Apply a color curve
// TODO option to output color
UCLASS(DisplayName = "Color Curve", Category = "Curve")
class VOXELGRAPH_API UVoxelNode_CurveColor : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (NonNull))
	UCurveLinearColor* Curve;

	UVoxelNode_CurveColor();

	virtual FText GetTitle() const override;
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Curve); }
};