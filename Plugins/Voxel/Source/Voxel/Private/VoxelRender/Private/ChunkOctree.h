#pragma once
#include "CoreMinimal.h"
#include "Octree.h"

class AVoxelChunk;
class VoxelRender;
class UVoxelInvokerComponent;

/**
 * Create the octree for rendering and spawn VoxelChunks
 */
class ChunkOctree : public Octree, public TSharedFromThis<ChunkOctree>
{
public:
	ChunkOctree(VoxelRender* Render, FIntVector Position, uint8 Depth, uint64 Id);


	VoxelRender* const Render;

	/**
	 * Unload VoxelChunk if created and recursively delete childs
	 */
	void Delete();

	/**
	 * Create/Update the octree for the new position
	 * @param	World			Current VoxelWorld
	 * @param	Invokers		List of voxel invokers
	 */
	void UpdateLOD(std::forward_list<TWeakObjectPtr<UVoxelInvokerComponent>> Invokers);

	/**
	 * Get a weak pointer to the leaf chunk at PointPosition. Weak pointers allow to check that the object they are pointing to is valid
	 * @param	PointPosition	Position in voxel space. Must be contained in this octree
	 * @return	Weak pointer to leaf chunk at PointPosition
	 */
	TWeakPtr<ChunkOctree> GetLeaf(FIntVector PointPosition);

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

	TWeakPtr<ChunkOctree> GetAdjacentChunk(TransitionDirection Direction);

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
	 */
	void Load();
	/**
	 * Unload the VoxelChunk
	 */
	void Unload();

	/**
	 * Create childs of this octree
	 */
	void CreateChilds();
	/**
	 * Delete childs (with their chunks)
	 */
	void DeleteChilds();
};

inline uint64 GetTypeHash(ChunkOctree ChunkOctree)
{
	return GetTypeHash(static_cast<Octree>(ChunkOctree));
}