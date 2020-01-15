// Copyright 2020 Phyronnaz

#include "VoxelNodes/VoxelRandomNodes.h"
#include "Runtime/VoxelNodeType.h"
#include "CppTranslation/VoxelVariables.h"
#include "VoxelContext.h"
#include "VoxelNodeFunctions.h"

UVoxelNode_RandomFloat::UVoxelNode_RandomFloat()
{
	SetInputs({"Seed", EC::Seed, "Seed"});
	SetOutputs(EC::Float);
}

FText UVoxelNode_RandomFloat::GetTitle() const
{
	return FText::FromString("Rand Float " + FString::SanitizeFloat(Min) + " " + FString::SanitizeFloat(Max));
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UVoxelNode_RandomInt::UVoxelNode_RandomInt()
{
	SetInputs({"Seed", EC::Seed, "Seed"});
	SetOutputs(EC::Int);
}

FText UVoxelNode_RandomInt::GetTitle() const
{
	return FText::FromString("Rand Int " + FString::FromInt(Min) + " " + FString::FromInt(Max));
}

