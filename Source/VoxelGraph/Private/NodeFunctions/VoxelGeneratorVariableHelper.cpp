// Copyright 2020 Phyronnaz

#include "NodeFunctions/VoxelGeneratorVariableHelper.h"
#include "VoxelGenerators/VoxelGeneratorCache.h"

void FVoxelGeneratorVariableHelper::Init(const FVoxelGeneratorInit& InitStruct) const
{
	Generator = InitStruct.GetGeneratorCache().MakeGeneratorInstance(Picker);
}