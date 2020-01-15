// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelSeedNodes.h"
#include "VoxelNodes/VoxelNodeColors.h"
#include "VoxelWorldGeneratorInit.h"
#include "VoxelGraphGlobals.h"

UVoxelSeedNode::UVoxelSeedNode()
{
	SetColor(FVoxelNodeColors::SeedNode);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_Seed::UVoxelNode_Seed()
{
	SetOutputs(EC::Seed);
}

FText UVoxelNode_Seed::GetTitle() const
{
	return FText::FromName(Name);
}

bool UVoxelNode_Seed::CanRenameNode() const
{
	return true;
}

FString UVoxelNode_Seed::GetEditableName() const
{
	return Name.IsNone() ? "" : Name.ToString();
}

void UVoxelNode_Seed::SetEditableName(const FString& NewName)
{
	Name = *NewName;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_AddSeeds::UVoxelNode_AddSeeds()
{
	SetInputs(EC::Seed);
	SetOutputs(EC::Seed);
	SetInputsCount(1, MAX_VOXELNODE_PINS);
}

