// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelExposedNodes.h"
#include "VoxelParameterNodes.generated.h"

// Float parameter
UCLASS(DisplayName = "float parameter", meta = (Keywords = "constant"))
class VOXELGRAPH_API UVoxelNode_FloatParameter : public UVoxelOptionallyExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	float Value;

	UVoxelNode_FloatParameter();
	
	//~ Begin UVoxelOptionallyExposedNode Interface
	virtual FString GetValueString() const override;
	virtual FLinearColor GetNotExposedColor() const override;
	//~ End UVoxelOptionallyExposedNode Interface

#if WITH_EDITOR
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object) override;
#endif
};

// Int parameter
UCLASS(DisplayName = "int parameter", meta = (Keywords = "constant"))
class VOXELGRAPH_API UVoxelNode_IntParameter : public UVoxelOptionallyExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 Value;

	UVoxelNode_IntParameter();

	//~ Begin UVoxelOptionallyExposedNode Interface
	virtual FString GetValueString() const override;
	virtual FLinearColor GetNotExposedColor() const override;
	//~ End UVoxelOptionallyExposedNode Interface

#if WITH_EDITOR
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object) override;
#endif
};

// Color parameter
UCLASS(DisplayName = "color parameter")
class VOXELGRAPH_API UVoxelNode_ColorParameter : public UVoxelOptionallyExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FLinearColor Color = FLinearColor(0, 0, 0, 1);

	UVoxelNode_ColorParameter();

	//~ Begin UVoxelOptionallyExposedNode Interface
	virtual FString GetValueString() const override;
	virtual FLinearColor GetNotExposedColor() const override;
	//~ End UVoxelOptionallyExposedNode Interface

#if WITH_EDITOR
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object) override;
#endif
};

// Bool parameter
UCLASS(DisplayName = "bool parameter", meta = (Keywords = "constant"))
class VOXELGRAPH_API UVoxelNode_BoolParameter : public UVoxelOptionallyExposedNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool Value;

	UVoxelNode_BoolParameter();

	//~ Begin UVoxelOptionallyExposedNode Interface
	virtual FString GetValueString() const override;
	virtual FLinearColor GetNotExposedColor() const override;
	//~ End UVoxelOptionallyExposedNode Interface

#if WITH_EDITOR
	virtual bool TryImportFromProperty(UProperty* Property, UObject* Object) override;
#endif
};