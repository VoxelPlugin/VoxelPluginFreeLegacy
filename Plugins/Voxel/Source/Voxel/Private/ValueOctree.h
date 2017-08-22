#pragma once
#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.h"

class VoxelData;
class UVoxelWorldGenerator;

/**
 * Octree that holds modified values & colors
 */
class ValueOctree
{
public:
	/**
	 * Constructor
	 * @param	Position		Position (center) of this in voxel space
	 * @param	Depth			Distance to the highest resolution
	 * @param	WorldGenerator	Generator of the current world
	 */
	ValueOctree(FIntVector Position, int Depth, UVoxelWorldGenerator* WorldGenerator);

	// Center of the octree
	const FIntVector Position;

	// Distance to the highest resolution
	const int Depth;


	/**
	 * Get the width at this level
	 * @return	Width of this chunk
	 */
	int Width() const;

	/**
	 * Does this chunk have no childs?
	 * @return	Whether or not this chunk has no childs
	 */
	bool IsLeaf() const;

	/**
	 * Does this chunk have been modified?
	 * @return	Whether or not this chunk is dirty
	 */
	bool IsDirty() const;

	/**
	 * Get value at position
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Value
	 */
	float GetValue(FIntVector GlobalPosition);
	/**
	 * Get color at position
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Color at position
	 */
	FColor GetColor(FIntVector GlobalPosition);

	/**
	 * Set value at position
	 * @param	GlobalPosition	Position in voxel space
	 * @param	Value to set
	 */
	void SetValue(FIntVector GlobalPosition, float Value);
	/**
	 * Set color at position
	 * @param	GlobalPosition	Position in voxel space
	 * @param	Color to set
	 */
	void SetColor(FIntVector GlobalPosition, FColor Color);

	/**
	 * Is GlobalPosition in this octree?
	 * @param	GlobalPosition	Position in voxel space
	 * @return	If IsInOctree	
	 */
	bool IsInOctree(FIntVector GlobalPosition) const;
	
	/**
	 * Convert from chunk space to voxel space
	 * @param	LocalPosition	Position in chunk space
	 * @return	Position in voxel space
	 */
	FIntVector LocalToGlobal(FIntVector LocalPosition) const;
	/**
	 * Convert from voxel space to chunk space
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Position in chunk space
	 */
	FIntVector GlobalToLocal(FIntVector GlobalPosition) const;

	/**
	 * Add dirty chunks to SaveArray
	 * @param	SaveArray	Array to save chunks into
	 */
	void AddChunksToArray(TArray<FVoxelChunkSaveStruct> SaveArray);
	/**
	 * Load chunks from SaveArray
	 * @param	SaveArray	Array to load chunks from
	 */
	void LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray);

	/**
	* Get direct child that owns GlobalPosition
	* @param	GlobalPosition	Position in voxel space
	*/
	ValueOctree* GetChild(FIntVector GlobalPosition);

private:
	/*
	Childs of this octree in the following order:
	
	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x
	*/
	TArray<ValueOctree*, TFixedAllocator<8>> Childs;

	// Generator for this world
	UVoxelWorldGenerator* WorldGenerator;

	// Values if dirty
	TArray<float, TFixedAllocator<16 * 16 * 16>> Values;
	// Colors if dirty
	TArray<FColor, TFixedAllocator<16 * 16 * 16>> Colors;

	bool bIsDirty;
	bool bIsLeaf;

	/**
	 * Create childs of this octree
	 */
	void CreateChilds();
};