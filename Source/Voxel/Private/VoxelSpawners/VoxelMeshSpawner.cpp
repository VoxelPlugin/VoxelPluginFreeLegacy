// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelMeshSpawner.h"
#include "VoxelSpawners/VoxelSpawnerGroup.h"
#include "VoxelData/VoxelData.h"
#include "VoxelData/VoxelDataAccelerator.h"
#include "VoxelVector.h"
#include "VoxelMessages.h"
#include "VoxelCustomVersion.h"
#include "VoxelWorldInterface.h"
#include "VoxelWorldGeneratorUtilities.h"

#include "Async/Async.h"
#include "TimerManager.h"
#include "Engine/StaticMesh.h"


void UVoxelMeshSpawnerBase::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	
	if (!IsTemplate())
	{
		ActorSettings.BodyInstance.FixupData(this);
		InstancedMeshSettings.BodyInstance.FixupData(this);
	}
}