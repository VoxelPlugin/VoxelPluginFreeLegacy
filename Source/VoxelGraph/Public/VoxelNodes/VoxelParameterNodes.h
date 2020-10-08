// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelExposedNodes.h"
#include "VoxelParameterNodes.generated.h"

// Float parameter
UCLASS(DisplayName = "float parameter", meta = (Keywords = "constant"))
class VOXELGRAPH_API UVoxelNode_FloatParameter : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float Value;

	UVoxelNode_FloatParameter();

	auto GetValue() const { return Value; }
	
	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Value); }
	//~ End UVoxelExposedNode Interface
};

// Int parameter
UCLASS(DisplayName = "int parameter", meta = (Keywords = "constant"))
class VOXELGRAPH_API UVoxelNode_IntParameter : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 Value;

	UVoxelNode_IntParameter();

	auto GetValue() const { return Value; }

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Value); }
	//~ End UVoxelExposedNode Interface
};

// Color parameter
UCLASS(DisplayName = "color parameter")
class VOXELGRAPH_API UVoxelNode_ColorParameter : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FLinearColor Color = FLinearColor(0, 0, 0, 1);

	UVoxelNode_ColorParameter();

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Color); }
	//~ End UVoxelExposedNode Interface
};

// Bool parameter
UCLASS(DisplayName = "bool parameter", meta = (Keywords = "constant"))
class VOXELGRAPH_API UVoxelNode_BoolParameter : public UVoxelExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool Value;

	UVoxelNode_BoolParameter();

	auto GetValue() const { return Value; }

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Value); }
	//~ End UVoxelExposedNode Interface
};