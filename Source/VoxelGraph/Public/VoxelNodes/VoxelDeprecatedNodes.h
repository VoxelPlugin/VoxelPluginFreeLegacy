// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelNodeHelper.h"
#include "VoxelGraphGlobals.h"
#include "VoxelNodeHelperMacros.h"
#include "VoxelGraphErrorReporter.h"

#include "VoxelGenerators/VoxelGeneratorPicker.h"

#include "VoxelNodes/VoxelExecNodes.h"
#include "VoxelNodes/VoxelExposedNodes.h"
#include "VoxelNodes/VoxelMaterialNodes.h"

#include "VoxelDeprecatedNodes.generated.h"

UCLASS(DisplayName = "Make Material From Single Index", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_MakeMaterialFromSingleIndex : public UVoxelMaterialNode
{
	GENERATED_BODY()
	DEPRECATED_VOXELNODE("Use Set Single Index")

	UVoxelNode_MakeMaterialFromSingleIndex()
	{
		SetInputs(EC::Int, EC::Float, EC::Float, EC::Float);
		SetOutputs(EC::Material);
	}
};

UCLASS(DisplayName = "Get Double Index", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_GetDoubleIndex : public UVoxelMaterialNode
{
	GENERATED_BODY()
	DEPRECATED_VOXELNODE("Use Multi Index")

	UVoxelNode_GetDoubleIndex()
	{
		SetInputs({ "Material", EC::Material, "Material" });
	SetOutputs(
		{ "Index A", EC::Int, "Index A between 0 and 255" },
		{ "Index B", EC::Int, "Index B between 0 and 255" },
		{ "Blend", EC::Float, "Blend factor, between 0 and 1" },
		{ "Data", EC::Float, "Data sent to material shader" });
	}
};

UCLASS(DisplayName = "Make Material From Color", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_MakeMaterialFromColor : public UVoxelMaterialNode
{
	GENERATED_BODY()
	DEPRECATED_VOXELNODE("Use SetColor")
	
		UVoxelNode_MakeMaterialFromColor()
	{
		SetInputs({ "Color", EC::Color, "Color" });
		SetOutputs(EC::Material);
	}
};

UCLASS(DisplayName = "Make Material From Double Index", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_MakeMaterialFromDoubleIndex : public UVoxelMaterialNode
{
	GENERATED_BODY()
	DEPRECATED_VOXELNODE("Use Multi Index")

	UVoxelNode_MakeMaterialFromDoubleIndex()
{
	SetInputs(
		{ "Index A", EC::Int, "Index A between 0 and 255", "", {0, 255} },
		{ "Index B", EC::Int, "Index B between 0 and 255", "", {0, 255} },
		{ "Blend", EC::Float, "Blend factor, between 0 and 1", "", {0, 1} },
		{ "Data", EC::Float, "Data to send to the material shader", "", {0, 1} });
	SetOutputs(EC::Material);
}
};

UCLASS(DisplayName = "Create Double Index Material", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_CreateDoubleIndexMaterial : public UVoxelMaterialNode
{
	GENERATED_BODY()
	DEPRECATED_VOXELNODE("Use Multi Index")

	UVoxelNode_CreateDoubleIndexMaterial()
	{
		SetInputsCount(3, MAX_VOXELNODE_PINS);
		SetInputIncrement(2);
		SetOutputs(EC::Material);
	}
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(DisplayName = "Set Double Index", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_SetDoubleIndex : public UVoxelNode_MaterialSetter
{
	GENERATED_BODY()
	DEPRECATED_VOXELNODE("Use MultiIndex nodes instead")

	UVoxelNode_SetDoubleIndex()
	{
		SetInputs(
			EC::Exec,
			{ "Index A", EC::Int, "Index A between 0 and 255", "", {0, 255} },
			{ "Index B", EC::Int, "Index B between 0 and 255", "", {0, 255} },
			{ "Blend", EC::Float, "Blend between 0 and 1", "", {0, 1} },
			{ "Data", EC::Float, "Data sent to material shader", "", {0, 1} });
		SetOutputs(EC::Exec);
	}
};

//////////////////////////////////////////////////////////////////////////////////////

UCLASS(DisplayName = "Generator Sampler", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_WorldGeneratorSampler : public UVoxelExposedNode
{
	GENERATED_BODY()
	DEPRECATED_VOXELNODE("use Get Generator Value / Get Generator Material instead")

public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelGeneratorPicker WorldGenerator;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TArray<FName> Seeds;
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

UCLASS(DisplayName = "Perlin Worm Distance", NotPlaceable)
class VOXELGRAPH_API UVoxelNode_PerlinWormDistance : public UVoxelNodeHelper
{
	GENERATED_BODY()
	DEPRECATED_VOXELNODE("Use DataItemSample instead")

	UVoxelNode_PerlinWormDistance()
	{
		SetInputs(
			{ "X", EC::Float, "X" },
			{ "Y", EC::Float, "Y" },
			{ "Z", EC::Float, "Z" });
		SetOutputs(EC::Float);
	}
};