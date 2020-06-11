// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"

class AVoxelWorld;
class UVoxelMaterialCollectionBase;

struct FVoxelWorldGeneratorInit
{
	TMap<FName, int32> Seeds;
	float VoxelSize = 100;
	uint32 WorldSize = 1 << 12;
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig(-1);
	const UVoxelMaterialCollectionBase* MaterialCollection = nullptr;
	const AVoxelWorld* World = nullptr; // Can be null

	FVoxelWorldGeneratorInit() {}

	FVoxelWorldGeneratorInit(
		const TMap<FName, int32>& Seeds,
		float VoxelSize,
		uint32 WorldSize,
		EVoxelMaterialConfig MaterialConfig,
		const UVoxelMaterialCollectionBase* MaterialCollection,
		const AVoxelWorld* World)
		: Seeds(Seeds)
		, VoxelSize(VoxelSize)
		, WorldSize(WorldSize)
		, MaterialConfig(MaterialConfig)
		, MaterialCollection(MaterialCollection)
		, World(World)
	{
	}
};