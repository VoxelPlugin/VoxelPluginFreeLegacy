// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.h"
#include "DiffStruct.h"
#include <list>

class ValueOctree;
class UVoxelWorldGenerator;

/**
 * Class that handle voxel data. Mainly an interface to ValueOctree
 */
class VoxelData
{
public:
	/**
	 * COnstructor
	 * @param	Depth			Depth of this world; Width = 16 * 2^Depth
	 * @param	WorldGenerator	Generator for this world
	 */
	VoxelData(int Depth, UVoxelWorldGenerator* WorldGenerator, bool bMultiplayer);

	// Width = 16 * 2^Depth
	const int Depth;

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
	 * Size of this world (== width)
	 * @return Size
	 */
	int Size() const;

	/**
	 * Get save array of this world
	 * @return SaveArray
	 */
	std::list<FVoxelChunkSaveStruct> GetSaveArray(bool bNetworkDirtyOnly = false) const;
	/**
	 * Load this world from save array
	 * @param	SaveArray	Array to load from
	 */
	void LoadFromArray(std::list<FVoxelChunkSaveStruct>& SaveArray) const;

	std::pair<std::forward_list<TArray<FVoxelValueDiff>>, std::forward_list<TArray<FVoxelColorDiff>>> GetDiffArrays() const;
	void LoadAndQueueUpdateFromDiffArray(const TArray<FVoxelValueDiff>& ValueDiffArray, const TArray<FVoxelColorDiff>& ColorDiffArray, AVoxelWorld* World) const;

private:
	// Values
	TSharedPtr<ValueOctree> MainOctree;
};
