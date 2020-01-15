// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelConstantNodes.generated.h"

// Returns the current LOD
UCLASS(DisplayName = "LOD", Category = "Constants")
class VOXELGRAPH_API UVoxelNode_LOD : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_LOD();
};

// Voxel Size
UCLASS(DisplayName = "Voxel Size", Category = "Constants")
class VOXELGRAPH_API UVoxelNode_VoxelSize : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_VoxelSize();
};

// World Size
UCLASS(DisplayName = "World Size", Category = "Constants")
class VOXELGRAPH_API UVoxelNode_WorldSize : public UVoxelNodeHelper
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_WorldSize();
};

// Use this to access compilation constants such as preview size, target...
UCLASS(DisplayName = "Compile-Time Constant", Category = "Constants", meta = (Keywords = "is"))
class VOXELGRAPH_API UVoxelNode_CompileTimeConstant : public UVoxelNodeHelper
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FName Name = "";

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelPinCategory Type = EVoxelPinCategory::Boolean;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Voxel")
	TMap<FName, FString> Constants;

	UVoxelNode_CompileTimeConstant();

	//~ Begin UVoxelNode Interface
	virtual FText GetTitle() const override;
	virtual EVoxelPinCategory GetOutputPinCategory(int32 PinIndex) const override;
	//~ End UVoxelNode Interface

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};