// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelChunkSaveStruct.h"

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
	VoxelData(int Depth, UVoxelWorldGenerator* WorldGenerator);
	virtual ~VoxelData();

	// Width = 16 * 2^Depth
	const int Depth;

	/**
	 * Get value at position
	 * @param	Position	Position in voxel space
	 * @return	Value (int between -127 and 127)
	 */
	signed char GetValue(FIntVector Position);
	/**
	 * Get color at position
	 * @param	Position	Position in voxel space
	 * @return	Color
	 */
	FColor GetColor(FIntVector Position);


	/**
	 * Set value at position
	 * @param	Position	Position in voxel space
	 * @param	Value to set (int between -127 and 127)
	 */
	void SetValue(FIntVector Position, int Value);
	/**
	 * Set color at position
	 * @param	Position	Position in voxel space
	 * @param	Color to set
	 */
	void SetColor(FIntVector Position, FColor Color);

	/**
	 * Is Position in this world?
	 * @param	Position	Position in voxel space
	 * @return	IsInWorld
	 */
	bool IsInWorld(FIntVector Position);

	
	/**
	 * Size of this world (== width)
	 * @return Size
	 */
	int Size();

	/**
	 * Get save array of this world
	 * @return SaveArray
	 */
	TArray<FVoxelChunkSaveStruct> GetSaveArray();
	/**
	 * Load this world from save array
	 * @param	SaveArray	Array to load from
	 */
	void LoadFromArray(TArray<FVoxelChunkSaveStruct> SaveArray);

private:
	// Values
	TSharedPtr<ValueOctree> MainOctree;
};
