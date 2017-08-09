#pragma once
#include "CoreMinimal.h"

class AVoxelChunk;
class AVoxelWorld;

/**
 * Create the octree for rendering and spawn VoxelChunks 
 */
class ChunkOctree : public TSharedFromThis<ChunkOctree>
{
public:
	/**
	 * Constructor
	 * @param	Position	Position (center) of this chunk
	 * @param	Depth		Distance to the highest resolution
	 */
	ChunkOctree(FIntVector Position, int Depth);

	bool operator==(const ChunkOctree& Other);

	// Center of the octree
	const FIntVector Position;

	// Distance to the highest resolution
	const int Depth;

	/**
	 * Unload VoxelChunk if created and recursively delete childs
	 */
	void Delete();

	/**
	 * Get the width at this level
	 * @return	Width of this chunk
	 */
	int Width();

	/**
	 * Create/Update the octree for the new position
	 * @param	World			Current VoxelWorld
	 * @param	CameraPosition	Position of the camera in world space
	 */
	void CreateTree(AVoxelWorld* World, FVector CameraPosition);

	/**
	 * Update VoxelChunks in this octree for changes in the terrain
	 * @param	bAsync	Update should be done async
	 */
	void Update(bool bAsync);

	/**
	 * Get a weak pointer to the leaf chunk at Position. Weak pointers allow to check that the object they are pointing to is valid
	 * @param	PointPosition	Position in voxel space. Must be contained in this octree
	 */
	TWeakPtr<ChunkOctree> GetChunk(FIntVector PointPosition);

	/**
	 * Get the VoxelChunk of this
	 * @return	VoxelChunk; can be nullptr
	 */
	AVoxelChunk* GetVoxelChunk();

private:
	/*
	Childs of this octree in the following order:
	
	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x
	*/
	TArray<TSharedPtr<ChunkOctree>, TFixedAllocator<8>> Childs;

	// Does this octree has childs?
	bool bHasChilds;
	// Is VoxelChunk created?
	bool bHasChunk;

	// Pointer to the chunk
	AVoxelChunk* VoxelChunk;

	/**
	 * Create the VoxelChunk
	 * @param	World	Current VoxelWorld
	 */
	void Load(AVoxelWorld* World);
	/**
	 * Unload the VoxelChunk
	 */
	void Unload();

	/**
	 * Create childs of this octree
	 */
	void CreateChilds();
	/**
	 * Call Delete() and reset all childs
	 */
	void DeleteChilds();
};