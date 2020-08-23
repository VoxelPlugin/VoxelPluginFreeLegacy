// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelBinaryNodes.generated.h"

// A < B
UCLASS(DisplayName = "float < float", Category = "Math|Float", meta = (Keywords = "< less"))
class VOXELGRAPH_API UVoxelNode_FLess : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("<")
	
	UVoxelNode_FLess();
};

// A <= B
UCLASS(DisplayName = "float <= float", Category = "Math|Float", meta = (Keywords = "<= less"))
class VOXELGRAPH_API UVoxelNode_FLessEqual : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("<=")
	
	UVoxelNode_FLessEqual();
};

// A > B
UCLASS(DisplayName = "float > float", Category = "Math|Float", meta = (Keywords = "> greater"))
class VOXELGRAPH_API UVoxelNode_FGreater : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE(">")
	
	UVoxelNode_FGreater();
};

// A >= B
UCLASS(DisplayName = "float >= float", Category = "Math|Float", meta = (Keywords = ">= greater"))
class VOXELGRAPH_API UVoxelNode_FGreaterEqual : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE(">=")
	
	UVoxelNode_FGreaterEqual();
};

// A == B
UCLASS(DisplayName = "float == float", Category = "Math|Float", meta = (Keywords = "== equal"))
class VOXELGRAPH_API UVoxelNode_FEqual : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("==")
	
	UVoxelNode_FEqual();
};

// A != B
UCLASS(DisplayName = "float != float", Category = "Math|Float", meta = (Keywords = "!= not equal"))
class VOXELGRAPH_API UVoxelNode_FNotEqual : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("!=")
	
	UVoxelNode_FNotEqual();
};

//////////////////////////////////////////////////////////////////////////////////////

// A < B
UCLASS(DisplayName = "int < int", Category = "Math|Integer", meta = (Keywords = "< less"))
class VOXELGRAPH_API UVoxelNode_ILess : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("<")
	
	UVoxelNode_ILess();
};

// A <= B
UCLASS(DisplayName = "int <= int", Category = "Math|Integer", meta = (Keywords = "<= less"))
class VOXELGRAPH_API UVoxelNode_ILessEqual : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("<=")
	
	UVoxelNode_ILessEqual();
};

// A > B
UCLASS(DisplayName = "int > int", Category = "Math|Integer", meta = (Keywords = "> greater"))
class VOXELGRAPH_API UVoxelNode_IGreater : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE(">")
	
	UVoxelNode_IGreater();
};

// A >= B
UCLASS(DisplayName = "int >= int", Category = "Math|Integer", meta = (Keywords = ">= greater"))
class VOXELGRAPH_API UVoxelNode_IGreaterEqual : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE(">=")
	
	UVoxelNode_IGreaterEqual();
};

// A == B
UCLASS(DisplayName = "int == int", Category = "Math|Integer", meta = (Keywords = "== equal"))
class VOXELGRAPH_API UVoxelNode_IEqual : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("==")
	
	UVoxelNode_IEqual();
};

// A != B
UCLASS(DisplayName = "int != int", Category = "Math|Integer", meta = (Keywords = "!= not equal"))
class VOXELGRAPH_API UVoxelNode_INotEqual : public UVoxelPureNode
{
	GENERATED_BODY()
	GENERATED_VOXELNODE_BODY()
	COMPACT_VOXELNODE("!=")
	
	UVoxelNode_INotEqual();
};