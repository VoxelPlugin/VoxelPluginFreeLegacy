#pragma once

#include "CoreMinimal.h"


void FluidStep(const int N, TArray<float>& Dens0, TArray<float>& U0, TArray<float>& V0, TArray<float>& W0, const float Visc, const float Diff, const float Dt, TArray<float>& Dens, TArray<float>& U, TArray<float>& V, TArray<float> W);