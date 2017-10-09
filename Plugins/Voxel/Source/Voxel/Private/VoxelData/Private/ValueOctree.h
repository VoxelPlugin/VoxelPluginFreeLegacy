#pragma once
#include "CoreMinimal.h"
#include "Octree.h"
#include "VoxelSave.h"
#include <list>
#include <forward_list>

class FVoxelData;
class IVoxelWorldGenerator;
struct FVoxelChunkSave;

/**
 * Octree that holds modified values & colors
 */
class FValueOctree : public FOctree
{
public:
	/**
	 * Constructor
	 * @param	Position		Position (center) of this in voxel space
	 * @param	Depth			Distance to the highest resolution
	 * @param	WorldGenerator	Generator of the current world
	 */
	FValueOctree(AVoxelWorldGenerator* WorldGenerator, FIntVector Position, uint8 Depth, uint64 Id);
	~FValueOctree();

	// Is the game multiplayer?
	bool bMultiplayer;

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
	void GetValueAndMaterial(int X, int Y, int Z, float& OutValue, FVoxelMaterial& OutMaterial);

	/**
	 * Set value at position
	 * @param	GlobalPosition	Position in voxel space
	 * @param	Value to set
	 */
	void SetValue(int X, int Y, int Z, float Value);

	/**
	 * Set color at position
	 * @param	GlobalPosition	Position in voxel space
	 * @param	Color to set
	 */
	void SetMaterial(int X, int Y, int Z, FVoxelMaterial Material);

	void SetValueAndMaterialNotThreadSafe(int X, int Y, int Z, float Value, FVoxelMaterial Material);

	/**
	 * Add dirty chunks to SaveList
	 * @param	SaveList		List to save chunks into
	 */
	void AddDirtyChunksToSaveList(std::list<TSharedRef<FVoxelChunkSave>>& SaveList);
	/**
	 * Load chunks from SaveArray
	 * @param	SaveArray	Array to load chunks from
	 */
	void LoadFromSaveAndGetModifiedPositions(std::list<FVoxelChunkSave>& Save, std::forward_list<FIntVector>& OutModifiedPositions);

	/**
	 * Add values that have changed since last network sync to diff arrays
	 * @param	ValuesDiffs		Values diff array; sorted by increasing Id
	 * @param	ColorsDiffs		Colors diff array; sorted by increasing Id
	 */
	void AddChunksToDiffArrays(VoxelValueDiffArray& ValuesDiffs, VoxelMaterialDiffArray& ColorsDiffs);
	/**
	 * Load values that have changed since last network sync from diff arrays
	 * @param	ValuesDiffs		Values diff array; top is lowest Id
	 * @param	ColorsDiffs		Colors diff array; top is lowest Id
	 * @param	World			Voxel world
	 */
	void LoadFromDiffListAndGetModifiedPositions(std::forward_list<FVoxelValueDiff>& ValuesDiffs, std::forward_list<FVoxelMaterialDiff>& ColorsDiffs, std::forward_list<FIntVector>& OutModifiedPositions);

	/**
	* Get direct child that owns GlobalPosition
	* @param	GlobalPosition	Position in voxel space
	*/
	FORCEINLINE FValueOctree* GetChild(int X, int Y, int Z);

	FValueOctree* GetLeaf(int X, int Y, int Z);

	/**
	 * Queue update of dirty chunks
	 * @param	World	Voxel world
	 */
	void GetDirtyChunksPositions(std::forward_list<FIntVector>& OutPositions);

private:
	/*
	Childs of this octree in the following order:

	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x
	*/
	TArray<FValueOctree*, TFixedAllocator<8>> Childs;

	FCriticalSection SetLock;

	// Values if dirty
	TArray<float, TFixedAllocator<16 * 16 * 16>> Values;
	// Materials if dirty
	TArray<FVoxelMaterial, TFixedAllocator<16 * 16 * 16>> Materials;

	bool bIsDirty;

	// For multiplayer

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

	FORCEINLINE int IndexFromCoordinates(int X, int Y, int Z);

	FORCEINLINE void CoordinatesFromIndex(int Index, int& OutX, int& OutY, int& OutZ);
};