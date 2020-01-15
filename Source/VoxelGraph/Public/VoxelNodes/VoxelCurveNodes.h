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
	UPROPERTY(EditAnywhere, Category = "Voxel")
	UCurveFloat* Curve;

	UVoxelNode_Curve();

	virtual FText GetTitle() const override;
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
#if WITH_EDITOR
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object) override;
#endif
};

// Apply a color curve
// TODO option to output color
UCLASS(DisplayName = "Color Curve", Category = "Curve")
class VOXELGRAPH_API UVoxelNode_CurveColor : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	UCurveLinearColor* Curve;

	UVoxelNode_CurveColor();

	virtual FText GetTitle() const override;
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
#if WITH_EDITOR
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object) override;
#endif
};