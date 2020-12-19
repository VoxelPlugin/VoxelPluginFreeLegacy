// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelEnums.h"
#include "VoxelMinimal.h"
#include "VoxelGeneratorInit.generated.h"

class AVoxelWorld;
class UVoxelMaterialCollectionBase;
class FVoxelGeneratorCache;
class IVoxelSpawnerManagerBase;

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelGeneratorInit
{
	GENERATED_BODY()

public:
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

	// Can be null. Needs to be a weak pointer, else the GC freaks out when exiting the map with an alive generator cache pointing to this 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
	TWeakObjectPtr<const AVoxelWorld> World;

	FVoxelGeneratorInit() = default;
	FVoxelGeneratorInit(
		float VoxelSize,
		uint32 WorldSize,
		EVoxelRenderType RenderType,
		EVoxelMaterialConfig MaterialConfig,
		const UVoxelMaterialCollectionBase* MaterialCollection,
		const TWeakObjectPtr<const AVoxelWorld>& World);

	TVoxelSharedRef<FVoxelGeneratorCache> GetGeneratorCache() const;
	TVoxelSharedPtr<IVoxelSpawnerManagerBase> GetSpawnerManager() const;
	
	void AddReferencedObjects(FReferenceCollector& Collector);

private:
	TVoxelWeakPtr<FVoxelGeneratorCache> GeneratorCache;
	TVoxelWeakPtr<IVoxelSpawnerManagerBase> SpawnerManager;

	friend class FVoxelGeneratorCache;

public:
	VOXEL_DEPRECATED(1.2, "Seeds are now regular generator parameters")
	TMap<FName, int32> Seeds;
};

VOXEL_DEPRECATED(1.2, "Use FVoxelGeneratorInit instead of FVoxelWorldGeneratorInit.")
typedef FVoxelGeneratorInit FVoxelWorldGeneratorInit;