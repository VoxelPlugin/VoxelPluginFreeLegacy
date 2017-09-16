#pragma once
#include "CoreMinimal.h"
#include "Octree.h"
#include "VoxelSave.h"
#include <list>
#include <forward_list>

class VoxelData;
class IVoxelWorldGenerator;
struct FVoxelChunkSave;

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
	ValueOctree(bool bMultiplayer, AVoxelWorldGenerator* WorldGenerator, FIntVector Position, uint8 Depth, uint64 Id = -1);

	// Is the game multiplayer?
	const bool bMultiplayer;

	// Generator for this world
	AVoxelWorldGenerator* WorldGenerator;

	/**
	 * Does this chunk have been modified?
	 * @return	Whether or not this chunk is dirty
	 */
	FORCEINLINE bool IsDirty() const;

	/**
	 * Get value and color at position
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Value
	 * @return	Color
	 */
	void GetValueAndColor(int X, int Y, int Z, float& OutValue, FColor& OutColor);

	/**
	 * Get value at position
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Value
	 */
	float GetValue(FIntVector GlobalPosition);

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
	void AddChunksToArray(std::list<FVoxelChunkSave>& SaveArray);
	/**
	 * Load chunks from SaveArray
	 * @param	SaveArray	Array to load chunks from
	 */
	void LoadAndQueueUpdateFromSave(std::list<FVoxelChunkSave>& SaveArray, AVoxelWorld* World);

	/**
	 * Add values that have changed since last network sync to diff arrays
	 * @param	ValuesDiffs		Values diff array; sorted by increasing Id
	 * @param	ColorsDiffs		Colors diff array; sorted by increasing Id
	 */
	void AddChunksToDiffArrays(VoxelValueDiffArray& ValuesDiffs, VoxelColorDiffArray& ColorsDiffs);
	/**
	 * Load values that have changed since last network sync from diff arrays
	 * @param	ValuesDiffs		Values diff array; top is lowest Id
	 * @param	ColorsDiffs		Colors diff array; top is lowest Id
	 * @param	World			Voxel world
	 */
	void LoadAndQueueUpdateFromDiffArrays(std::forward_list<FVoxelValueDiff>& ValuesDiffs, std::forward_list<FVoxelColorDiff>& ColorsDiffs, AVoxelWorld* World);

	/**
	* Get direct child that owns GlobalPosition
	* @param	GlobalPosition	Position in voxel space
	*/
	ValueOctree* GetChild(FIntVector GlobalPosition);
	FORCEINLINE ValueOctree* GetChild(int X, int Y, int Z);

	/**
	 * Queue update of dirty chunks
	 * @param	World	Voxel world
	 */
	void QueueUpdateOfDirtyChunks(AVoxelWorld* World);

	TSharedPtr<ValueOctree> GetCopy();

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

	FCriticalSection CriticalSection;

	// Values if dirty
	TArray<float, TFixedAllocator<16 * 16 * 16>> Values;
	// Colors if dirty
	TArray<FColor, TFixedAllocator<16 * 16 * 16>> Colors;

	bool bIsDirty;

	TSet<int> DirtyValues;
	TSet<int> DirtyColors;

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