// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "VoxelSpawners/VoxelFoliage.h"
#include "ThumbnailRendering/BlueprintThumbnailRenderer.h"
#include "ThumbnailRendering/StaticMeshThumbnailRenderer.h"
#include "VoxelFoliageThumbnailRenderer.generated.h"

UCLASS()
class VOXELEDITOR_API UVoxelFoliageThumbnailRenderer : public UStaticMeshThumbnailRenderer
{
	GENERATED_BODY()

public:
	virtual bool CanVisualizeAsset(UObject* Object) override
	{
		return GetMesh(Object) || GetMutableDefault<UBlueprintThumbnailRenderer>()->CanVisualizeAsset(Object);
	}
	virtual void GetThumbnailSize(UObject* Object, float Zoom, uint32& OutWidth, uint32& OutHeight) const override
	{
		if (auto* Mesh = GetMesh(Object))
		{
			return Super::GetThumbnailSize(Mesh, Zoom, OutWidth, OutHeight);
		}
		else
		{
			return GetMutableDefault<UBlueprintThumbnailRenderer>()->GetThumbnailSize(Object, Zoom, OutWidth, OutHeight);
		}
	}
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* Target, FCanvas* Canvas ONLY_UE_25_AND_HIGHER(, bool bAdditionalViewFamily)) override
	{
		if (auto* Mesh = GetMesh(Object))
		{
			return Super::Draw(Mesh, X, Y, Width, Height, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
		}
		else
		{
			return GetMutableDefault<UBlueprintThumbnailRenderer>()->Draw(Object, X, Y, Width, Height, Target, Canvas ONLY_UE_25_AND_HIGHER(, bAdditionalViewFamily));
		}
	}

private:
	UStaticMesh* GetMesh(UObject* Object) const
	{
		auto* Blueprint = Cast<UBlueprint>(Object);
		if (!Blueprint)
		{
			return nullptr;
		}

		UClass* GeneratedClass = Blueprint->GeneratedClass;
		if (!GeneratedClass ||
			!GeneratedClass->IsChildOf(UVoxelFoliage::StaticClass()))
		{
			return nullptr;
		}

		UVoxelFoliage* CDO = GeneratedClass->GetDefaultObject<UVoxelFoliage>();
		if (!CDO)
		{
			return nullptr;
		}

		return CDO->StaticMesh;
	}
};