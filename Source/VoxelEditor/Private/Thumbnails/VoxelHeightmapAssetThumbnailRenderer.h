// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "VoxelAssets/VoxelHeightmapAsset.h"
#include "VoxelHeightmapAssetThumbnailRenderer.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelHeightmapAssetThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override
	{
		return Object->IsA(UVoxelHeightmapAsset::StaticClass());
	}
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override
	{
		UTextureThumbnailRenderer::GetThumbnailSize(CastChecked<UVoxelHeightmapAsset>(Object)->GetThumbnail(), Zoom, OutWidth, OutHeight);
	}
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas, bool bAdditionalViewFamily) override
	{
		UTextureThumbnailRenderer::Draw(CastChecked<UVoxelHeightmapAsset>(Object)->GetThumbnail(), X, Y, Width, Height, Target, Canvas, bAdditionalViewFamily);
	}
};