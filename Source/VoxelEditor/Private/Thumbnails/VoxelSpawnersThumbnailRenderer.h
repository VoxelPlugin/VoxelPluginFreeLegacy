// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "ThumbnailRendering/StaticMeshThumbnailRenderer.h"
#include "VoxelSpawnersThumbnailRenderer.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelMeshSpawnerThumbnailRenderer : public UStaticMeshThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily)) override;
};

UCLASS()
class VOXELEDITOR_API UVoxelAssetSpawnerThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily)) override;
};

UCLASS()
class VOXELEDITOR_API UVoxelSpawnerGroupThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily)) override;
};

UCLASS()
class VOXELEDITOR_API UVoxelMeshSpawnerGroupThumbnailRenderer : public UStaticMeshThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily)) override;
};