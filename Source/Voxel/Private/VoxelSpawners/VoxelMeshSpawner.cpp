// Copyright 2020 Phyronnaz

#include "VoxelSpawners/VoxelMeshSpawner.h"
#include "Engine/StaticMesh.h"

void UVoxelMeshSpawner::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	
	if (!IsTemplate())
	{
		ActorSettings.BodyInstance.FixupData(this);
		InstancedMeshSettings.BodyInstance.FixupData(this);
	}
}

#if WITH_EDITOR
bool UVoxelMeshSpawner::NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const
{
	return Object == Mesh;
}
#endif