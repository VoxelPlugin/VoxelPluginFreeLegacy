// Copyright 2020 Phyronnaz

#include "FastNoise/VoxelFastNoiseLUT.h"
#include "FastNoise/CrossPlatformSTD.h"

#include <random>

void FVoxelFastNoiseLUT::SetSeed(int32 NewSeed)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	Seed = NewSeed;

	std::mt19937 Generator(NewSeed);

	// There is a bug below:
	// 256 - j should be 255 - j
	// However, fixing it would break all existing generators
	// Instead, make sure it's deterministic
	Perm.Memzero();
	Perm12.Memzero();
	
	for (int32 Index = 0; Index < 256; Index++)
	{
		Perm[Index] = Index;
	}

	for (int32 j = 0; j < 256; j++)
	{
		cross_platform_std::uniform_int_distribution<> Distribution(0, 256 - j); // <- bug here
		// K should be max 255
		// Due to the bug it's max 256
		// m_perm.Num() = 512, so it's fine to do m_perm[k]
		int32 k = Distribution(Generator) + j;
		int32 l = Perm[j];
		Perm[j] = Perm[j + 256] = Perm[k];
		Perm[k] = l;
		Perm12[j] = Perm12[j + 256] = Perm[j] % 12;
	}
}