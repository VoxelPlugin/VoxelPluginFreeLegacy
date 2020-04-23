// Copyright 2020 Phyronnaz

#include "VoxelWorldGenerator.h"
#include "VoxelWorldGeneratorInstance.h"
#include "VoxelMessages.h"

TMap<FName, int32> UVoxelWorldGenerator::GetDefaultSeeds() const
{
	FVoxelMessages::Info("This generator does not support GetDefaultSeeds", this);
	return {};
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelWorldGenerator::GetInstance()
{
	unimplemented();
	return TVoxelSharedPtr<FVoxelWorldGeneratorInstance>().ToSharedRef();
}

///////////////////////////////////////////////////////////////////////////////

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelTransformableWorldGenerator::GetTransformableInstance()
{
	unimplemented();
	return TVoxelSharedPtr<FVoxelTransformableWorldGeneratorInstance>().ToSharedRef();
}

void UVoxelTransformableWorldGenerator::SaveInstance(const FVoxelTransformableWorldGeneratorInstance& Instance, FArchive& Ar) const
{
	unimplemented();
}

TVoxelSharedRef<FVoxelTransformableWorldGeneratorInstance> UVoxelTransformableWorldGenerator::LoadInstance(FArchive& Ar) const
{
	unimplemented();
	return TVoxelSharedPtr<FVoxelTransformableWorldGeneratorInstance>().ToSharedRef();
}

TVoxelSharedRef<FVoxelWorldGeneratorInstance> UVoxelTransformableWorldGenerator::GetInstance()
{
	return GetTransformableInstance();
}

///////////////////////////////////////////////////////////////////////////////

FIntBox UVoxelTransformableWorldGeneratorWithBounds::GetBounds() const
{
	unimplemented();
	return {};
}