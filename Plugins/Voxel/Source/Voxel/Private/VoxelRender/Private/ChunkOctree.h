#pragma once
#include "CoreMinimal.h"
#include "Octree.h"

class UVoxelChunkComponent;
class FVoxelRender;
class UVoxelInvokerComponent;

/**
 * Create the octree for rendering and spawn VoxelChunks
 */
class FChunkOctree : public FOctree, public TSharedFromThis<FChunkOctree>
{
public:
	FChunkOctree(FVoxelRender* Render, FIntVector Position, uint8 Depth, uint64 Id);


	FVoxelRender* const Render;

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
	TWeakPtr<FChunkOctree> GetLeaf(FIntVector PointPosition);

	/**
	 * Get the VoxelChunk of this
	 * @return	VoxelChunk; can be nullptr
	 */
	UVoxelChunkComponent* GetVoxelChunk() const;

	/**
	* Get direct child at position. Must not be leaf
	* @param	PointPosition	Position in voxel space. Must be contained in this octree
	* @return	Direct child in which PointPosition is contained
	*/
	TSharedPtr<FChunkOctree> GetChild(FIntVector PointPosition);

	TWeakPtr<FChunkOctree> GetAdjacentChunk(TransitionDirection Direction);

private:
	/*
	Childs of this octree in the following order:

	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x
	*/
	TArray<TSharedPtr<FChunkOctree>, TFixedAllocator<8>> Childs;

	// Is VoxelChunk created?
	bool bHasChunk;

	// Pointer to the chunk
	UVoxelChunkComponent* VoxelChunk;

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

inline uint64 GetTypeHash(FChunkOctree ChunkOctree)
{
	return GetTypeHash(static_cast<FOctree>(ChunkOctree));
}