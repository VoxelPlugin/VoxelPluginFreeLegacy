// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"

namespace FVoxelMath
{
	inline int DivideFloor(int Dividend, int Divisor)
	{
		int Q = Dividend / Divisor;
		int R = Dividend % Divisor;
		if ((R != 0) && ((R < 0) != (Divisor < 0)))
		{
			Q--;
		}
		return Q;
	}
}