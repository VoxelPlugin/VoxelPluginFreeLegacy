// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelDataAssetThumbnailRenderer.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelDataAssetThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override
	{
		return Object->IsA(UVoxelDataAsset::StaticClass());
	}
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override
	{
		UTextureThumbnailRenderer::GetThumbnailSize(CastChecked<UVoxelDataAsset>(Object)->GetThumbnail(), Zoom, OutWidth, OutHeight);
	}
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily)) override
	{
		UTextureThumbnailRenderer::Draw(CastChecked<UVoxelDataAsset>(Object)->GetThumbnail(), X, Y, Width, Height, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
	}
};