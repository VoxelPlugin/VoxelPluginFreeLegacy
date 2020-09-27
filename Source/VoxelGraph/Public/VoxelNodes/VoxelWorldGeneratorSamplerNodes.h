// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelExposedNodes.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorPicker.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelWorldGeneratorSamplerNodes.generated.h"

UCLASS(Abstract, Category = "World Generator")
class VOXELGRAPH_API UVoxelNode_WorldGeneratorSamplerBase : public UVoxelExposedNode
{
	GENERATED_BODY()

public:
	// Seeds to send to the world generators
	UPROPERTY(EditAnywhere, Category = "Config", meta = (ReconstructNode))
	TArray<FName> Seeds;

	//~ Begin UVoxelNode Interface
	virtual EVoxelPinCategory GetInputPinCategory(int32 PinIndex) const override;
	virtual FName GetInputPinName(int32 PinIndex) const override;
	virtual int32 GetMinInputPins() const override;
	virtual int32 GetMaxInputPins() const override;
	//~ End UVoxelNode Interface
};

UCLASS(Abstract)
class VOXELGRAPH_API UVoxelNode_SingleWorldGeneratorSamplerBase : public UVoxelNode_WorldGeneratorSamplerBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	FVoxelWorldGeneratorPicker WorldGenerator;

	UVoxelNode_SingleWorldGeneratorSamplerBase();
	
	//~ Begin UVoxelNode Interface
	virtual void LogErrors(FVoxelGraphErrorReporter& ErrorReporter) override;
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface

#if WITH_EDITOR
	//~ Begin UVoxelExposedNode Interface
	virtual bool TryImportFromProperty(FProperty* Property, UObject* Object) override;
	//~ End UVoxelExposedNode Interface
#endif
};

UCLASS(DisplayName = "Get World Generator Value")
class VOXELGRAPH_API UVoxelNode_GetWorldGeneratorValue : public UVoxelNode_SingleWorldGeneratorSamplerBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetWorldGeneratorValue();
};

UCLASS(DisplayName = "Get World Generator Material")
class VOXELGRAPH_API UVoxelNode_GetWorldGeneratorMaterial : public UVoxelNode_SingleWorldGeneratorSamplerBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetWorldGeneratorMaterial();
};

UCLASS(DisplayName = "Get World Generator Custom Output")
class VOXELGRAPH_API UVoxelNode_GetWorldGeneratorCustomOutput : public UVoxelNode_SingleWorldGeneratorSamplerBase
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GetWorldGeneratorCustomOutput();

	UPROPERTY(EditAnywhere, Category = "Config")
	FName OutputName = "Value";

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	//~ End UVoxelNode Interface
};