// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

namespace FVoxelUtilities
{
	// Delay until next fire; 0 means "next frame"
	VOXEL_API void DelayedCall(TFunction<void()> Call, float Delay = 0);
}