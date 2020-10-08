// Copyright 2020 Phyronnaz

#include "VoxelGenerators/VoxelGeneratorPicker.h"
#include "VoxelGenerators/VoxelEmptyGenerator.h"
#include "VoxelMessages.h"

#include "UObject/Package.h"

TVoxelSharedRef<FVoxelGeneratorInstance> FVoxelGeneratorPicker::GetInstance(bool bSilent) const
{
	VOXEL_FUNCTION_COUNTER();

	auto* Generator = GetGenerator();
	if (Generator)
	{
		return Generator->GetInstance(Parameters);
	}
	else
	{
		FVoxelMessages::CondError(!bSilent, FUNCTION_ERROR("Invalid generator"));
		return MakeVoxelShared<FVoxelEmptyGeneratorInstance>();
	}
}

TVoxelSharedRef<FVoxelTransformableGeneratorInstance> FVoxelTransformableGeneratorPicker::GetInstance(bool bSilent) const
{
	VOXEL_FUNCTION_COUNTER();
	
	auto* Generator = GetGenerator();
	if (Generator)
	{
		return Generator->GetTransformableInstance(Parameters);
	}
	else
	{
		FVoxelMessages::CondError(!bSilent, FUNCTION_ERROR("Invalid generator"));
		return MakeVoxelShared<FVoxelTransformableEmptyGeneratorInstance>();
	}
}