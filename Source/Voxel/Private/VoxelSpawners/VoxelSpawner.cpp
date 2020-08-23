// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelSpawner.h"
#include "VoxelSpawners/VoxelMeshSpawner.h"
#include "VoxelSpawners/VoxelAssetSpawner.h"
#include "VoxelSpawners/VoxelSpawnerGroup.h"


bool FVoxelSpawnersSaveImpl::Serialize(FArchive& Ar)
{
	if ((Ar.IsLoading() || Ar.IsSaving()) && !Ar.IsTransacting())
	{
		if (Ar.IsSaving())
		{
			Version = FVoxelSpawnersSaveVersion::LatestVersion;
		}

		Ar << Version;
		Ar << Guid;
		Ar << CompressedData;
	}

	return true;
}