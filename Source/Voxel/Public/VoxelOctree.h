// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"
#include "VoxelId.h"

template<class ElementType, uint32 ChunkSize>
class TVoxelOctree
{
public:
	// Center of the octree
	const FIntVector Position;

	// Distance to the highest resolution
	const uint8 LOD;

	// Id of the Octree (position in the octree)
	const FVoxelId Id;

	const FIntBox OctreeBounds;

	TVoxelOctree(uint8 LOD)
		: Position(FIntVector::ZeroValue)
		, LOD(LOD)
		, Id(EForceInit::ForceInitToZero)
		, OctreeBounds(GetCachedBounds())
	{
		checkVoxelSlow(LOD < MAX_WORLD_DEPTH);
	}
	virtual ~TVoxelOctree()
	{
		if (!IsLeaf())
		{
			DestroyChildren();
		}
	}

	TVoxelOctree(const TVoxelOctree&) = delete;
	TVoxelOctree(const TVoxelOctree&&) = delete;
    TVoxelOctree& operator=(const TVoxelOctree&) = delete;

public:
	/**
	 * Get the width at this level
	 * @return	Width of this chunk
	 */
	inline int Size() const
	{
		return ChunkSize << LOD;
	}

	inline const FIntBox& GetBounds() const
	{
		return OctreeBounds;
	}

	/**
	 * Is Leaf?
	 * @return IsLeaf
	 */
	inline bool IsLeaf() const
	{
		return !Children;
	}

	/**
	 * Is the given Id a child of this octree?
	 */
	inline bool IsIdChild(const FVoxelId& ChildId) const
	{
		return FVoxelId::IsChild(Id, ChildId);
	}

	template<typename TNumeric>
	inline bool IsInOctree(TNumeric X, TNumeric Y, TNumeric Z) const
	{
		return GetBounds().IsInside(X, Y, Z);
	}
	template<typename TVector>
	inline bool IsInOctree(const TVector& P) const
	{
		return IsInOctree(P.X, P.Y, P.Z);
	}

	/**
	 * Convert from chunk space to voxel space
	 * @param	LocalPosition	Position in chunk space
	 * @return	Position in voxel space
	 */
	template<typename TNumeric>
	inline void LocalToGlobal(TNumeric X, TNumeric Y, TNumeric Z, TNumeric& OutX, TNumeric& OutY, TNumeric& OutZ) const
	{
		OutX = X + OctreeBounds.Min.X;
		OutY = Y + OctreeBounds.Min.Y;
		OutZ = Z + OctreeBounds.Min.Z;
	}

	/**
	 * Convert from voxel space to chunk space
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Position in chunk space
	 */
	template<typename TNumeric>
	inline void GlobalToLocal(TNumeric X, TNumeric Y, TNumeric Z, TNumeric& OutX, TNumeric& OutY, TNumeric& OutZ) const
	{
		OutX = X - OctreeBounds.Min.X;
		OutY = Y - OctreeBounds.Min.Y;
		OutZ = Z - OctreeBounds.Min.Z;
	}
	
public:
	template<typename TNumeric>
	inline const ElementType& GetChild(TNumeric X, TNumeric Y, TNumeric Z) const
	{
		return GetChild(GetChildIndex(X, Y, Z));
	}
	template<typename TNumeric>
	inline ElementType& GetChild(TNumeric X, TNumeric Y, TNumeric Z)
	{
		return GetChild(GetChildIndex(X, Y, Z));
	}

	template<typename TVector>
	inline const ElementType& GetChild(const TVector& P) const
	{
		return GetChild(P.X, P.Y, P.Z);
	}
	template<typename TVector>
	inline ElementType& GetChild(const TVector& P)
	{
		return GetChild(P.X, P.Y, P.Z);
	}

	inline const ElementType& GetChild(int Index) const
	{
		checkVoxelSlow((Children != nullptr) & (0 <= Index) & (Index < 8));
		return Children[Index];
	}
	inline ElementType& GetChild(int Index)
	{
		checkVoxelSlow((Children != nullptr) & (0 <= Index) & (Index < 8));
		return Children[Index];
	}

	using FChildrenArray = ElementType[8];

	inline const FChildrenArray& GetChildren() const
	{
		checkVoxelSlow(Children);
		return reinterpret_cast<const FChildrenArray&>(*Children);
	}
	inline FChildrenArray& GetChildren()
	{
		checkVoxelSlow(Children);
		return reinterpret_cast<FChildrenArray&>(*Children);
	}

public:
	/**
	 * Get the octree leaf at global position
	 * @param	X,Y,Z	Global Position
	 */
	template<typename TNumeric>
	inline ElementType* GetLeaf(TNumeric X, TNumeric Y, TNumeric Z) const
	{
		checkVoxelSlow(IsInOctree(X, Y, Z));

		const ElementType* Ptr = static_cast<const ElementType*>(this);

		while (!Ptr->IsLeaf())
		{
			Ptr = &Ptr->GetChild(X, Y, Z);
		}

		checkVoxelSlow(Ptr->IsInOctree(X, Y, Z));

		return const_cast<ElementType*>(Ptr);
	}
	inline ElementType* GetLeaf(const FIntVector& P) const
	{
		return GetLeaf(P.X, P.Y, P.Z);
	}
	
	void GetLeavesOverlappingBox(const FIntBox& Box, TArray<ElementType*>& OutOctrees)
	{
		if (GetBounds().Intersect(Box))
		{
			if (IsLeaf())
			{
				OutOctrees.Add(static_cast<ElementType*>(this));
			}
			else
			{
				for (auto& Child : GetChildren())
				{
					Child.GetLeavesOverlappingBox(Box, OutOctrees);
				}
			}
		}
	}

	template<typename F>
	void ApplyLambda(F Lambda)
	{
		Lambda(static_cast<ElementType*>(this));

		if (!IsLeaf())
		{
			for (auto& Child : GetChildren())
			{
				Child.ApplyLambda(Lambda);
			}
		}
	}

	template<typename TContainer>
	void GetLeavesBounds(TContainer& OutBounds, int LeafMaxLOD = MAX_WORLD_DEPTH) const
	{
		if (IsLeaf())
		{
			if (LOD <= LeafMaxLOD)
			{
				OutBounds.Add(GetBounds());
			}
		}
		else
		{
			for (auto& Child : GetChildren())
			{
				Child.template GetLeavesBounds<TContainer>(OutBounds, LeafMaxLOD);
			}
		}
	}

	template<template <typename...> class TContainer, typename... TArgs>
	void GetLeaves(TContainer<ElementType*, TArgs...>& OutOctrees, int LeafMaxLOD = MAX_WORLD_DEPTH)
	{
		if (IsLeaf())
		{
			if (LOD <= LeafMaxLOD)
			{
				OutOctrees.Add(static_cast<ElementType*>(this));
			}
		}
		else
		{
			for (auto& Child : GetChildren())
			{
				Child.template GetLeaves<TContainer>(OutOctrees, LeafMaxLOD);
			}
		}
	}

	// Warning: OutOctrees is reversed
	void GetLeavesWithIds(TArray<FVoxelId>& Ids, TArray<ElementType*>& OutOctrees)
	{
		if (Ids.Num() == 0)
		{
			return;
		}

		if (LOD == 0)
		{
			while (Ids.Num() > 0 && Ids.Last() == Id)
			{
				OutOctrees.Add(static_cast<ElementType*>(this));
				Ids.Pop(false);
			}
		}
		else
		{
			if (IsIdChild(Ids.Last()))
			{
				if (IsLeaf())
				{
					while (Ids.Num() > 0 && IsIdChild(Ids.Last()))
					{
						OutOctrees.Add(static_cast<ElementType*>(this));
						Ids.Pop(false);
					}
				}
				else
				{
					for (auto& Child : GetChildren())
					{
						Child.GetLeavesWithIds(Ids, OutOctrees);
					}
				}
			}
		}
	}

	inline ElementType* CreateLeafAt(const FIntVector& InPosition)
	{
		auto* Octree = this;
		while (Octree->LOD != 0)
		{
			if (Octree->IsLeaf())
			{
				static_cast<ElementType*>(Octree)->CreateChildren();
			}
			Octree = &Octree->GetChild(InPosition);
		}
		return static_cast<ElementType*>(Octree);
	}

protected:
	TVoxelOctree(ElementType* Parent, uint8 ChildIndex)
		: Position(Parent->Position +
			FIntVector(
				Parent->Size() / 4 * ((ChildIndex & 1) ? 1 : -1),
				Parent->Size() / 4 * ((ChildIndex & 2) ? 1 : -1),
				Parent->Size() / 4 * ((ChildIndex & 4) ? 1 : -1)
			)
		)
		, LOD(Parent->LOD - 1)
		, Id(FVoxelId::FromParent(Parent->Id, Parent->LOD - 1, ChildIndex + 1))
		, OctreeBounds(GetCachedBounds())
	{
		checkVoxelSlow(0 <= ChildIndex && ChildIndex < 8);
	}

	/**
	 * Create childs of this octree
	 */
	template<typename... TArgs>
	inline void CreateChildren(TArgs&&... Args)
	{		
		checkVoxelSlow(IsLeaf());
		checkVoxelSlow(!Children);
		checkVoxelSlow(LOD != 0);

		Children = (ElementType*)FMemory::Malloc(8 * sizeof(ElementType));

		for (int Index = 0; Index < 8 ; Index++)
		{
			new (&Children[Index]) ElementType(static_cast<ElementType*>(this), Index, Forward<TArgs>(Args)...);
		}
	}

	inline void DestroyChildren()
	{
		checkVoxelSlow(!IsLeaf());

		for (auto& Child : GetChildren())
		{
			Child.~ElementType();
		}

		FMemory::Free(Children);
		Children = nullptr;
	}


private:
	/*
	Children of this octree in the following order:

	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x
	*/
	ElementType* Children = nullptr;

	inline FIntBox GetCachedBounds() const
	{
		return FIntBox(Position - FIntVector(Size() / 2), Position + FIntVector(Size() / 2));
	}

	template<typename TNumeric>
	inline int GetChildIndex(TNumeric X, TNumeric Y, TNumeric Z) const
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		return (X >= Position.X) + 2 * (Y >= Position.Y) + 4 * (Z >= Position.Z);
	}
};