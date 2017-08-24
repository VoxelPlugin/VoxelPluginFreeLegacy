#pragma once
#include "CoreMinimal.h"
#include "VoxelChunk.h"

inline uint32 IntPow9(int Power)
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
	Octree(FIntVector Position, int Depth, int Id = -1) : Position(Position), Depth(Depth), Id(Id == -1 ? IntPow9(Depth) : Id), bHasChilds(false)
	{
		// TODO: Use bigger int
		// Max for Id
		check(Depth <= 9);
	};

	bool operator==(const Octree& Other) const
	{
		check((Id == Other.Id) == (Position == Other.Position && Depth == Other.Depth));
		return Id == Other.Id;
	}

	bool operator<(const Octree& Other) const
	{
		return Id < Other.Id;
	}

	bool operator>(const Octree& Other) const
	{
		return Id > Other.Id;
	}

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
	int Width() const
	{
		return 16 << Depth;
	}

	/**
	 * Is Leaf?
	 * @return IsLeaf
	 */
	bool IsLeaf() const
	{
		return !bHasChilds;
	}

	/**
	 * Is GlobalPosition in this octree?
	 * @param	GlobalPosition	Position in voxel space
	 * @return	If IsInOctree
	 */
	bool IsInOctree(FIntVector GlobalPosition) const
	{
		return Position.X - Width() / 2 <= GlobalPosition.X && GlobalPosition.X < Position.X + Width() / 2 &&
			Position.Y - Width() / 2 <= GlobalPosition.Y && GlobalPosition.Y < Position.Y + Width() / 2 &&
			Position.Z - Width() / 2 <= GlobalPosition.Z && GlobalPosition.Z < Position.Z + Width() / 2;
	}

	/**
	 * Convert from chunk space to voxel space
	 * @param	LocalPosition	Position in chunk space
	 * @return	Position in voxel space
	 */
	FIntVector LocalToGlobal(FIntVector LocalPosition) const
	{
		return FIntVector(LocalPosition.X + (Position.X - Width() / 2), LocalPosition.Y + (Position.Y - Width() / 2), LocalPosition.Z + (Position.Z - Width() / 2));
	}

	/**
	 * Convert from voxel space to chunk space
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Position in chunk space
	 */
	FIntVector GlobalToLocal(FIntVector GlobalPosition) const
	{
		return FIntVector(GlobalPosition.X - (Position.X - Width() / 2), GlobalPosition.Y - (Position.Y - Width() / 2), GlobalPosition.Z - (Position.Z - Width() / 2));
	}

protected:
	// Does this octree has childs?
	bool bHasChilds;
};

inline uint32 GetTypeHash(Octree Octree)
{
	return Octree.Id;
}