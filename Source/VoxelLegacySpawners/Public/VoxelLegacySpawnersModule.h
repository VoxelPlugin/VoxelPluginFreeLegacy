// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

class UVoxelFoliage;
class UVoxelFoliageCollection;
class UVoxelSpawnerGroup;
class UVoxelSpawnerConfig;
class UVoxelMeshSpawner;
class UVoxelMeshSpawnerBase;
class UVoxelMeshSpawnerGroup;
class UVoxelLegacySpawnerBase;

struct FVoxelSpawnerDensity;
struct FVoxelFoliageDensity;

class FVoxelLegacySpawnersModule : public IModuleInterface
{
public:
	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	//~ End IModuleInterface Interface

private:
	struct FAssetToConvert
	{
		UClass* OldClass = nullptr;
		UClass* NewClass = nullptr;

		TFunction<void(const UObject*, UObject*)> Convert;
	};
	TArray<FAssetToConvert> AssetsToConvert;

	UObject* Convert(const UVoxelLegacySpawnerBase* OldObject) const;
	
	void Convert_UVoxelMeshSpawner_UVoxelFoliage(const UVoxelMeshSpawner& MeshSpawner, UVoxelFoliage& Foliage) const;
	void Convert_UVoxelMeshSpawnerGroup_UVoxelFoliage(const UVoxelMeshSpawnerGroup& MeshSpawner, UVoxelFoliage& Foliage) const;
	void Convert_UVoxelSpawnerGroup_UVoxelFoliage(const UVoxelSpawnerGroup& SpawnerGroup, UVoxelFoliage& Foliage) const;

	void Convert_UVoxelMeshSpawnerBase_UVoxelFoliage(const UVoxelMeshSpawnerBase& MeshSpawner, UVoxelFoliage& Foliage) const;

	void Convert_UVoxelSpawnerConfig_UVoxelFoliageCollection(const UVoxelSpawnerConfig& SpawnerConfig, UVoxelFoliageCollection& FoliageCollection) const;
	void Convert_FVoxelSpawnerDensity_FVoxelFoliageDensity(const FVoxelSpawnerDensity& OldDensity, FVoxelFoliageDensity& NewDensity) const;
};
