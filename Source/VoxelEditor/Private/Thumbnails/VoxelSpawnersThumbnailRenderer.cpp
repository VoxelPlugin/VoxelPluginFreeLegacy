// Copyright 2020 Phyronnaz

#include "VoxelSpawnersThumbnailRenderer.h"
#include "VoxelSpawners/VoxelMeshSpawner.h"

#include "Engine/StaticMesh.h"
#include "ThumbnailRendering/ThumbnailManager.h"

bool UVoxelMeshSpawnerThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	return Object->IsA(UVoxelMeshSpawner::StaticClass()) && CastChecked<UVoxelMeshSpawner>(Object)->Mesh;
}

void UVoxelMeshSpawnerThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily))
{
	UStaticMeshThumbnailRenderer::Draw(CastChecked<UVoxelMeshSpawner>(Object)->Mesh, X, Y, Width, Height, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
}