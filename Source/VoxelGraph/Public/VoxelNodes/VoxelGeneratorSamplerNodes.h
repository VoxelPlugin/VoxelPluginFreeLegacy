// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelExposedNodes.h"
#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelGeneratorSamplerNodes.generated.h"

UCLASS(Abstract, Category = "Generator")
class VOXELGRAPH_API UVoxelNode_GeneratorSamplerBase : public UVoxelExposedNode
{
	GENERATED_BODY()

public:
	//~ Begin UVoxelNode Interface
	//~ End UVoxelNode Interface
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_SingleGeneratorSamplerBase : public UVoxelNode_GeneratorSamplerBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelGeneratorPicker Generator;

	UVoxelNode_SingleGeneratorSamplerBase();
	
	//~ Begin UVoxelNode Interface
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface

	//~ Begin UVoxelExposedNode Interface
	virtual FName GetParameterPropertyName() const override { return GET_OWN_MEMBER_NAME(Generator); }
	//~ End UVoxelExposedNode Interface
};

UCLASS(DisplayName = "Get Generator Value")
class VOXELGRAPH_API UVoxelNode_GetGeneratorValue : public UVoxelNode_SingleGeneratorSamplerBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetGeneratorValue();
};

UCLASS(DisplayName = "Get Generator Material")
class VOXELGRAPH_API UVoxelNode_GetGeneratorMaterial : public UVoxelNode_SingleGeneratorSamplerBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetGeneratorMaterial();
};

UCLASS(DisplayName = "Get Generator Custom Output")
class VOXELGRAPH_API UVoxelNode_GetGeneratorCustomOutput : public UVoxelNode_SingleGeneratorSamplerBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetGeneratorCustomOutput();

	UPROPERTY(EditAnywhere, Category = "Config")
	FName OutputName = "Value";

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface
};