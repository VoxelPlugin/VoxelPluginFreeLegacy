#pragma once

#include "CoreMinimal.h"

FIntVector operator*(int left, FIntVector right)
{
	return right * left;
}