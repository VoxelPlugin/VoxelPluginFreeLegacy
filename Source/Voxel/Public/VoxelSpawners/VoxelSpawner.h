// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelMinimal.h"
#include "VoxelSaveStruct.h"
#include "VoxelSpawner.generated.h"

class FVoxelConstDataAccelerator;
class FVoxelSpawnerManager;
class FVoxelSpawnerProxy;
class FVoxelData;
class AVoxelSpawnerActor;
class UVoxelSpawner;

namespace FVoxelSpawnersSaveVersion
{
	enum Type : int32
	{
		BeforeCustomVersionWasAdded,
		SHARED_PlaceableItemsInSave,
		SHARED_AssetItemsImportValueMaterials,
		SHARED_DataAssetScale,
		SHARED_RemoveVoxelGrass,
		SHARED_DataAssetTransform,
		SHARED_RemoveEnableVoxelSpawnedActorsEnableVoxelGrass,
		SHARED_FoliagePaint,
		SHARED_ValueConfigFlagAndSaveGUIDs,
		SHARED_SingleValues,
		SHARED_NoVoxelMaterialInHeightmapAssets,
		SHARED_FixMissingMaterialsInHeightmapAssets,
		SHARED_AddUserFlagsToSaves,
		StoreSpawnerMatricesRelativeToComponent,
		SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage,
		
		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
}

struct VOXEL_API FVoxelSpawnersSaveImpl
{	
	FVoxelSpawnersSaveImpl() = default;

	bool Serialize(FArchive& Ar);

	bool operator==(const FVoxelSpawnersSaveImpl& Other) const
	{
		return Guid == Other.Guid;
	}
	
private:
	// Version of FVoxelSpawnerSave, not of the compressed data!
	int32 Version;
	FGuid Guid;
	TArray<uint8> CompressedData;

	friend class FVoxelSpawnerManager;
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UCLASS(Abstract)
class VOXEL_API UVoxelSpawner : public UObject
{
	GENERATED_BODY()

public:
	// Average distance between the instances, in voxels
	// Num Instances = Area in voxels / Square(DistanceBetweenInstancesInVoxel)
	// Not a density because the values would be too small to store in a float
	UPROPERTY(EditAnywhere, Category = "General Settings", meta = (ClampMin = 0))
	float DistanceBetweenInstancesInVoxel = 10;

	// Use this if you create the spawner at runtime
	UPROPERTY(Transient)
	uint32 SeedOverride = 0;
	
public:
#if WITH_EDITOR
	virtual bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) { return false; }
#endif
	
};

USTRUCT(BlueprintType, Category = Voxel)
struct VOXEL_API FVoxelSpawnersSave
#if CPP
	: public TVoxelSaveStruct<FVoxelSpawnersSaveImpl>
#endif
{	
	GENERATED_BODY()
};

DEFINE_VOXEL_SAVE_STRUCT(FVoxelSpawnersSave)