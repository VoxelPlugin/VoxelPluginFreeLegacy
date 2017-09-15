#pragma once
#include "CoreMinimal.h"
#include "Octree.h"

class AVoxelChunk;
class AVoxelWorld;

/**
 * Create the octree for rendering and spawn VoxelChunks
 */
class ChunkOctree : public Octree, public TSharedFromThis<ChunkOctree>
{
public:
	ChunkOctree(FIntVector Position, uint8 Depth, uint64 Id = -1) : Octree(Position, Depth, Id), bHasChunk(false), VoxelChunk(nullptr)
	{
	};

	/**
	 * Unload VoxelChunk if created and recursively delete childs
	 */
	void Delete();
	/**
	 * No deletion delay
	 */
	void ImmediateDelete();

	/**
	 * Create/Update the octree for the new position
	 * @param	World			Current VoxelWorld
	 * @param	CameraPosition	Position of the camera in world space
	 */
	void UpdateCameraPosition(AVoxelWorld* World, FVector CameraPosition);

	/**
	 * Update VoxelChunks in this octree for changes in the terrain
	 * @param	bAsync	Update should be done async
	 */
	void Update(bool bAsync);

	/**
	 * Get a weak pointer to the leaf chunk at PointPosition. Weak pointers allow to check that the object they are pointing to is valid
	 * @param	PointPosition	Position in voxel space. Must be contained in this octree
	 * @return	Weak pointer to leaf chunk at PointPosition
	 */
	TWeakPtr<ChunkOctree> GetChunk(FIntVector PointPosition);

	/**
	 * Get the VoxelChunk of this
	 * @return	VoxelChunk; can be nullptr
	 */
	AVoxelChunk* GetVoxelChunk() const;

	/**
	* Get direct child at position. Must not be leaf
	* @param	PointPosition	Position in voxel space. Must be contained in this octree
	* @return	Direct child in which PointPosition is contained
	*/
	TSharedPtr<ChunkOctree> GetChild(FIntVector PointPosition);

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

inline uint64 GetTypeHash(ChunkOctree ChunkOctree)
{
	return GetTypeHash(static_cast<Octree>(ChunkOctree));
}