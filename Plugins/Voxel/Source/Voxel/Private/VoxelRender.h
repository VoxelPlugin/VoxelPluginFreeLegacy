#pragma once

#include "CoreMinimal.h"
#include "ValueOctree.h"
#include "ProceduralMeshComponent.h"

class VoxelRender
{
public:
	VoxelRender(int Depth, ValueOctree* Octree);

	void CreateSection(FProcMeshSection& OutSection);

private:
	const int Depth;

	uint64 Signs[64];

	// Caches to get index of already created vertices
	int Cache1[16][16][3];
	int Cache2[16][16][3];
	// Allows to ping-pong between 2 caches
	bool bNewCacheIs1;

	ValueOctree* Octree;

	FORCEINLINE int Width();
	// Step between cubes
	FORCEINLINE int Step();
};