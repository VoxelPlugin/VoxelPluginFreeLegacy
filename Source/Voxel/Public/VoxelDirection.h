// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

namespace EVoxelDirectionFlag
{
	enum Type : uint8
	{
		XMin = 0x01,
		XMax = 0x02,
		YMin = 0x04,
		YMax = 0x08,
		ZMin = 0x10,
		ZMax = 0x20
	};
}