#pragma once
#include "CoreMinimal.h"
#include "Octree.h"
#include "DiffStruct.h"
#include <list>
#include <forward_list>

class VoxelData;
class UVoxelWorldGenerator;
struct FVoxelChunkSaveStruct;

/**
 * Octree that holds modified values & colors
 */
class ValueOctree : public Octree
{
public:
	/**
	 * Constructor
	 * @param	Position		Position (center) of this in voxel space
	 * @param	Depth			Distance to the highest resolution
	 * @param	WorldGenerator	Generator of the current world
	 */
	ValueOctree(bool bNetworking, UVoxelWorldGenerator* WorldGenerator, FIntVector Position, int Depth, int Id = -1) : Octree(Position, Depth, Id),
		bNetworking(bNetworking), WorldGenerator(WorldGenerator), bIsDirty(false), bIsNetworkDirty(false), bSyncAllValues(false), bSyncAllColors(false)
	{
		check(WorldGenerator);
	};

	const bool bNetworking;

	static int SyncAllThreshold;

	/**
	 * Does this chunk have been modified?
	 * @return	Whether or not this chunk is dirty
	 */
	bool IsDirty() const
	{
		return bIsDirty;
	}

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
	 * Add dirty chunks to SaveArray
	 * @param	SaveArray		List to save chunks into
	 */
	void AddChunksToArray(std::list<FVoxelChunkSaveStruct>& SaveArray);
	/**
	 * Load chunks from SaveArray
	 * @param	SaveArray	Array to load chunks from
	 */
	void LoadFromArray(std::list<FVoxelChunkSaveStruct>& SaveArray);

	void AddChunksToDiffStruct(DiffSaveStruct& DiffStruct);

	void LoadAndQueueUpdateFromDiffArray(std::forward_list<FSingleDiffStruct>& DiffArray, AVoxelWorld* World);

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

	TSet<int> DirtyValues;
	TSet<int> DirtyColors;
	bool bSyncAllValues;
	bool bSyncAllColors;

	bool bIsNetworkDirty;

	/**
	 * Create childs of this octree
	 */
	void CreateChilds();

	/**
	 * Init arrays
	 */
	void SetAsDirty();
};