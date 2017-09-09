#pragma once
#include "CoreMinimal.h"
#include "VoxelChunk.h"

FORCEINLINE uint32 IntPow9(int Power)
{
	check(Power <= 10);
	int Pow = 1;
	for (int i = 0; i < Power; i++)
	{
		Pow *= 9;
	}
	return Pow;
}

/**
 * Base Octree class
 */
class Octree
{
public:
	/**
	 * Constructor
	 * @param	Position	Position (center) of this chunk
	 * @param	Depth		Distance to the highest resolution
	 */
	Octree(FIntVector Position, int Depth, int Id = -1);

	bool operator==(const Octree& Other) const;

	bool operator<(const Octree& Other) const;

	bool operator>(const Octree& Other) const;

	// Center of the octree
	const FIntVector Position;

	// Distance to the highest resolution
	const int Depth;

	// Id of the Octree (position in the octree)
	const uint32 Id;

	/**
	 * Get the width at this level
	 * @return	Width of this chunk
	 */
	FORCEINLINE int Width() const;

	/**
	 * Is Leaf?
	 * @return IsLeaf
	 */
	FORCEINLINE bool IsLeaf() const;

	/**
	 * Is GlobalPosition in this octree?
	 * @param	GlobalPosition	Position in voxel space
	 * @return	If IsInOctree
	 */
	bool IsInOctree(FIntVector GlobalPosition) const;

	FORCEINLINE bool IsInOctree(int X, int Y, int Z) const;

	/**
	 * Convert from chunk space to voxel space
	 * @param	LocalPosition	Position in chunk space
	 * @return	Position in voxel space
	 */
	FIntVector LocalToGlobal(FIntVector LocalPosition) const;

	FORCEINLINE void LocalToGlobal(int X, int Y, int Z, int& OutX, int& OutY, int& OutZ) const;

	/**
	 * Convert from voxel space to chunk space
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Position in chunk space
	 */
	FIntVector GlobalToLocal(FIntVector GlobalPosition) const;

	FORCEINLINE void GlobalToLocal(int X, int Y, int Z, int& OutX, int& OutY, int& OutZ) const;

protected:
	// Does this octree has child?
	bool bHasChilds;
};

inline uint32 GetTypeHash(Octree Octree)
{
	return Octree.Id;
}