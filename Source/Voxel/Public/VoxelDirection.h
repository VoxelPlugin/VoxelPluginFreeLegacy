// Copyright 2019 Phyronnaz

#pragma once

enum EVoxelDirection : uint8
{
	XMin = 0x1,
	XMax = 0x2,
	YMin = 0x4,
	YMax = 0x8,
	ZMin = 0x10,
	ZMax = 0x20
};

inline EVoxelDirection InverseVoxelDirection(EVoxelDirection Direction)
{
	switch (Direction)
	{
	case XMin:
		return XMax;
	case XMax:
		return XMin;
	case YMin:
		return YMax;
	case YMax:
		return YMin;
	case ZMin:
		return ZMax;
	case ZMax:
		return ZMin;
	default:
		check(false);
		return XMin;
	}
}