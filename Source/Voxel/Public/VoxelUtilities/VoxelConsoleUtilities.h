// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

class AVoxelWorld;

namespace FVoxelUtilities
{
	VOXEL_API FConsoleCommandWithWorldAndArgsDelegate CreateVoxelWorldCommandWithArgs(TFunction<void(AVoxelWorld& World, const TArray<FString>& Args)> Lambda);
	VOXEL_API FConsoleCommandWithWorldDelegate CreateVoxelWorldCommand(TFunction<void(AVoxelWorld& World)> Lambda);
}