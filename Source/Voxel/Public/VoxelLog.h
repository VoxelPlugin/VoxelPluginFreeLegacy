// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

VOXEL_API DECLARE_LOG_CATEGORY_EXTERN(LogVoxel, Log, All);

#define LOG_VOXEL(Verbosity, Format, ...) UE_LOG(LogVoxel, Verbosity, Format, ##__VA_ARGS__)