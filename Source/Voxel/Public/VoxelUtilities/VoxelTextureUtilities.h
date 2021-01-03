// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class UTexture2D;

namespace FVoxelTextureUtilities
{
	VOXEL_API void UpdateColorTexture(UTexture2D*& Texture, const FIntPoint& Size, const TArray<FColor>& Colors);
}