// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"
#include "VoxelGraphGenerator.h"
#include "ThumbnailRendering/TextureThumbnailRenderer.h"
#include "VoxelGraphGeneratorThumbnailRenderer.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelGraphGeneratorThumbnailRenderer : public UTextureThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override
	{
		return Object->IsA(UVoxelGraphGenerator::StaticClass());
	}
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override
	{
		UTextureThumbnailRenderer::GetThumbnailSize(CastChecked<UVoxelGraphGenerator>(Object)->GetPreviewTexture(), Zoom, OutWidth, OutHeight);
	}
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas, bool bAdditionalViewFamily) override
	{
		UTextureThumbnailRenderer::Draw(CastChecked<UVoxelGraphGenerator>(Object)->GetPreviewTexture(), X, Y, Width, Height, Target, Canvas, bAdditionalViewFamily);
	}
};