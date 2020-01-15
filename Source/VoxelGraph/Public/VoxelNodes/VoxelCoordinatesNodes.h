// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelCoordinatesNodes.generated.h"

UCLASS(Abstract, Category = "Coordinates")
class VOXELGRAPH_API UVoxelCoordinateNode : public UVoxelNodeWithDependencies
{
	GENERATED_BODY()

public:
	UVoxelCoordinateNode();
};

//////////////////////////////////////////////////////////////////////////////////////

// Return the current X
UCLASS(DisplayName = "X", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_XF : public UVoxelCoordinateNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_XF();
	virtual uint8 GetNodeDependencies() const override;
};

// Return the current Y
UCLASS(DisplayName = "Y", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_YF : public UVoxelCoordinateNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_YF();
	virtual uint8 GetNodeDependencies() const override;
};

// Return the current Z
UCLASS(DisplayName = "Z", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_ZF : public UVoxelCoordinateNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_ZF();
	virtual uint8 GetNodeDependencies() const override;
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(DisplayName = "X (int)", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_XI : public UVoxelNodeHelper
{
	GENERATED_BODY()
	SET_VOXELNODE_TITLE("X")
	DEPRECATED_VOXELNODE("please use the float version instead")
	
	UVoxelNode_XI() { SetOutputs(EC::Int); }
};

UCLASS(DisplayName = "Y (int)", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_YI : public UVoxelNodeHelper
{
	GENERATED_BODY()
	SET_VOXELNODE_TITLE("Y")
	DEPRECATED_VOXELNODE("please use the float version instead")

	UVoxelNode_YI() { SetOutputs(EC::Int); }
};

UCLASS(DisplayName = "Z (int)", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_ZI : public UVoxelNodeHelper
{
	GENERATED_BODY()
	SET_VOXELNODE_TITLE("Z")
	DEPRECATED_VOXELNODE("please use the float version instead")
	
	UVoxelNode_ZI() { SetOutputs(EC::Int); }
};

//////////////////////////////////////////////////////////////////////////////////////

// Return the current X in global space, before the transform is applied to it. Same as X if not a graph asset
UCLASS(DisplayName = "Global X", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_GlobalX : public UVoxelCoordinateNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GlobalX();
	virtual uint8 GetNodeDependencies() const override;
};

// Return the current Y in global space, before the transform is applied to it. Same as Y if not a graph asset
UCLASS(DisplayName = "Global Y", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_GlobalY : public UVoxelCoordinateNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GlobalY();
	virtual uint8 GetNodeDependencies() const override;
};

// Return the current Z in global space, before the transform is applied to it. Same as Z if not a graph asset
UCLASS(DisplayName = "Global Z", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_GlobalZ : public UVoxelCoordinateNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GlobalZ();
	virtual uint8 GetNodeDependencies() const override;
};

//////////////////////////////////////////////////////////////////////////////////////

// Transform coordinates from local voxel space to global voxel space. Used for graph assets
UCLASS(DisplayName = "Local To Global", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_LocalToGlobal : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_LocalToGlobal();
};

// Transform coordinates from global voxel space to local voxel space. Used for graph assets
UCLASS(DisplayName = "Global To Local", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_GlobalToLocal : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_GlobalToLocal();
};

// Transform vector from local voxel space to global voxel space. Used for graph assets
UCLASS(DisplayName = "Transform Vector", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_TransformVector : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_TransformVector();
};

// Transform vector from global voxel space to local voxel space. Used for graph assets
UCLASS(DisplayName = "Inverse Transform Vector", Category = "Coordinates")
class VOXELGRAPH_API UVoxelNode_InverseTransformVector : public UVoxelNodeWithContext
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()

	UVoxelNode_InverseTransformVector();
};