// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSave.h"
#include <list>

class ValueOctree;
class IVoxelWorldGenerator;

/**
 * Class that handle voxel data. Mainly an interface to ValueOctree
 */
class VoxelData
{
public:
	/**
	 * Constructor
	 * @param	Depth			Depth of this world; Width = 16 * 2^Depth
	 * @param	WorldGenerator	Generator for this world
	 */
	VoxelData(int Depth, AVoxelWorldGenerator* WorldGenerator, bool bMultiplayer);

	// Depth of the octree
	const int Depth;

	// Width = 16 * 2^Depth
	FORCEINLINE int Width() const;

	/**
	 * Get value octree
	 * @return	Value Octree
	 */
	TSharedPtr<ValueOctree> GetValueOctree() const;

	/**
	 * Get value at position
	 * @param	Position	Position in voxel space
	 * @return	Value
	 */
	float GetValue(FIntVector Position) const;

	/**
	 * Get color at position
	 * @param	Position	Position in voxel space
	 * @return	Color
	 */
	FColor GetColor(FIntVector Position) const;

	/**
	* Get value and color at position
	* @param	Position	Position in voxel space
	* @return	Value
	* @return	Color
	*/
	void GetValueAndColor(int X, int Y, int Z, float& OutValue, FColor& OutColor) const;


	/**
	 * Set value at position
	 * @param	Position	Position in voxel space
	 * @param	Value to set
	 */
	void SetValue(FIntVector Position, float Value) const;
	/**
	 * Set color at position
	 * @param	Position	Position in voxel space
	 * @param	Color to set
	 */
	void SetColor(FIntVector Position, FColor Color) const;

	/**
	 * Is Position in this world?
	 * @param	Position	Position in voxel space
	 * @return	IsInWorld
	 */
	bool IsInWorld(FIntVector Position) const;

	/**
	 * Get save array of this world
	 * @return SaveArray
	 */
	FVoxelWorldSave GetSave() const;
	/**
	 * Load this world from save array
	 * @param	SaveArray	Array to load from
	 * @param	World		VoxelWorld
	 * @param	bReset		Reset all chunks?
	 */
	void LoadAndQueueUpdateFromSave(std::list<FVoxelChunkSave>& SaveArray, AVoxelWorld* World, bool bReset);

	/**
	 * Get sliced diff arrays to allow network transmission
	 * @param	OutValueDiffPacketsList		Each packet is sorted by Id
	 * @param	OutColorDiffPacketsList		Each packet is sorted by Id
	 */
	void GetDiffArrays(std::forward_list<TArray<FVoxelValueDiff>>& OutValueDiffPacketsList, std::forward_list<TArray<FVoxelColorDiff>>& OutColorDiffPacketsList) const;
	/**
	 * Load values and colors from diff arrays, and queue update of chunks that have changed
	 * @param	ValueDiffArray	First element has lowest Id
	 * @param	ColorDiffArray	First element has lowest Id
	 * @param	World			Voxel world
	 */
	void LoadAndQueueUpdateFromDiffArray(const TArray<FVoxelValueDiff>& ValueDiffArray, const TArray<FVoxelColorDiff>& ColorDiffArray, AVoxelWorld* World) const;

private:
	// Values
	TSharedPtr<ValueOctree> MainOctree;
};
