// Copyright 2020 Phyronnaz

#include "VoxelData/VoxelDataOctree.h"
#include "VoxelData/VoxelDataIncludes.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelDataOctreesMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelUndoRedoMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelMultiplayerMemory);
DEFINE_STAT(STAT_VoxelDataOctreesCount);

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelDataOctreeDirtyValuesMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelDataOctreeDirtyMaterialsMemory);

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelDataOctreeCachedValuesMemory);
DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelDataOctreeCachedMaterialsMemory);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDataOctreeParent::CreateChildren()
{
	TVoxelOctreeParent::CreateChildren();

#if DO_THREADSAFE_CHECKS
	for (auto& Child : AsParent().GetChildren())
	{
		Child.Parent = this;
	}
#endif

	if (ItemHolder->NumItems() > 0)
	{
		for (auto& Child : AsParent().GetChildren())
		{
			const FVoxelIntBox ChildBounds = Child.GetBounds();
			ItemHolder->ApplyToAllItems([&](auto& Item)
			{
				if (Item.Bounds.Intersect(ChildBounds))
				{
					Child.ItemHolder->AddItem(Item);
				}
			});
		}
	}
	ItemHolder.Reset();
}

void FVoxelDataOctreeParent::DestroyChildren()
{
	TVoxelOctreeParent::DestroyChildren();

	check(!ItemHolder.IsValid());
	// Always valid on a node with no children
	ItemHolder = MakeUnique<FVoxelPlaceableItemHolder>();
}