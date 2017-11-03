#pragma once

#include "CoreMinimal.h"


void FluidStep(const int N, TArray<float> PrevDens, TArray<float> PrevU, TArray<float> PrevV, const float Visc, const float Diff, const float Dt, TArray<float>& OutDens, TArray<float>& OutU, TArray<float>& OutV);