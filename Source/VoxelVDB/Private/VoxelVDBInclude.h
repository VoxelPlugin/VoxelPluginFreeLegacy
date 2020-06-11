// Copyright 2020 Phyronnaz

#pragma once

#include "VoxelMinimal.h"

THIRD_PARTY_INCLUDES_START
#pragma warning(push)
#pragma warning(disable: 4146)

#include <openvdb/openvdb.h>
#include <openvdb/io/Stream.h>
#include <openvdb/tools/Interpolation.h>

#include "OpenVDB7/FindActiveValues.h"

#pragma warning(pop)
THIRD_PARTY_INCLUDES_END