// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelConstantNodes.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelContext.h"
#include "VoxelNodeFunctions.h"
#include "VoxelGraphGenerator.h"
#include "VoxelWorldGeneratorInit.h"

UVoxelNode_LOD::UVoxelNode_LOD()
{
	SetOutputs(EC::Int);
}
GENERATED_VOXELNODE_IMPL
(
	UVoxelNode_LOD,
	NO_INPUTS,
	DEFINE_OUTPUTS(int32),
	_O0 = _C0.LOD;
)

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_VoxelSize::UVoxelNode_VoxelSize()
{
	SetOutputs(EC::Float);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_WorldSize::UVoxelNode_WorldSize()
{
	SetOutputs(EC::Int);
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_CompileTimeConstant::UVoxelNode_CompileTimeConstant()
{
	SetOutputs(EC::Boolean);
}

FText UVoxelNode_CompileTimeConstant::GetTitle() const
{
	return FText::FromName(Name);
}

EVoxelPinCategory UVoxelNode_CompileTimeConstant::GetOutputPinCategory(int32 PinIndex) const
{
	return Type;
}


#if WITH_EDITOR
void UVoxelNode_CompileTimeConstant::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (Graph && GraphNode && PropertyChangedEvent.Property && PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive)
	{
		GraphNode->ReconstructNode();
		Graph->CompileVoxelNodesFromGraphNodes();
	}
}
#endif