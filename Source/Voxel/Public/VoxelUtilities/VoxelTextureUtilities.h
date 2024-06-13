// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UTexture2D;

namespace FVoxelTextureUtilities
{
	VOXEL_API void UpdateColorTexture(TObjectPtr<UTexture2D>& Texture, const FIntPoint& Size, const TArray<FColor>& Colors);
}