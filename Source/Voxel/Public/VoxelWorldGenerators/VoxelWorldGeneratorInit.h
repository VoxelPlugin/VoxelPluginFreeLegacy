// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelWorldGeneratorInit.generated.h"

class AVoxelWorld;
class UVoxelMaterialCollectionBase;

USTRUCT(BlueprintType)
struct FVoxelWorldGeneratorInit
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	TMap<FName, int32> Seeds;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	float VoxelSize = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	int32 WorldSize = 1 << 12;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	EVoxelRenderType RenderType = EVoxelRenderType::MarchingCubes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	EVoxelMaterialConfig MaterialConfig = EVoxelMaterialConfig::RGB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	const UVoxelMaterialCollectionBase* MaterialCollection = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	const AVoxelWorld* World = nullptr; // Can be null

	FVoxelWorldGeneratorInit() = default;
	FVoxelWorldGeneratorInit(
		const TMap<FName, int32>& Seeds,
		float VoxelSize,
		uint32 WorldSize,
		EVoxelRenderType RenderType,
		EVoxelMaterialConfig MaterialConfig,
		const UVoxelMaterialCollectionBase* MaterialCollection,
		const AVoxelWorld* World)
		: Seeds(Seeds)
		, VoxelSize(VoxelSize)
		, WorldSize(WorldSize)
		, RenderType(RenderType)
		, MaterialConfig(MaterialConfig)
		, MaterialCollection(MaterialCollection)
		, World(World)
	{
	}
};