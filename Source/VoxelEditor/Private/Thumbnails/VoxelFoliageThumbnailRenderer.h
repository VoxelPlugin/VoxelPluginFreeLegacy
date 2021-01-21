// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMesh.h"
#include "VoxelFoliage.h"
#include "ThumbnailRendering/StaticMeshThumbnailRenderer.h"
#include "VoxelFoliageThumbnailRenderer.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelFoliageThumbnailRenderer : public UStaticMeshThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override
	{
		return Object->IsA<UVoxelFoliage>();
	}
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override
	{
		return Super::GetThumbnailSize(GetMesh(CastChecked<UVoxelFoliage>(Object)), Zoom, OutWidth, OutHeight);
	}
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily)) override
	{
		return Super::Draw(GetMesh(CastChecked<UVoxelFoliage>(Object)), X, Y, Width, Height, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
	}

private:
	static UStaticMesh* GetMesh(UVoxelFoliage* Foliage)
	{
		return Foliage->Meshes.Num() > 0 ? Foliage->Meshes[0].Mesh : nullptr;
	}
};