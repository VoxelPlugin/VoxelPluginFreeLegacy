// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelSpawners/VoxelMeshSpawner.h"
#include "VoxelSpawners/VoxelSpawnerGroup.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelVector.h"
#include "VoxelMessages.h"
#include "VoxelWorldInterface.h"
#include "VoxelUtilities/VoxelGeneratorUtilities.h"

#include "Async/Async.h"
#include "TimerManager.h"
#include "Engine/StaticMesh.h"

#if WITH_EDITOR
bool UVoxelMeshSpawner::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent)
{
	return Object == Mesh;
}

bool UVoxelMeshSpawnerGroup::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent)
{
	return Meshes.Contains(Object);
}
#endif


void UVoxelMeshSpawnerBase::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	
	if (!IsTemplate())
	{
		ActorSettings.BodyInstance.FixupData(this);
		InstancedMeshSettings.BodyInstance.FixupData(this);
	}
}