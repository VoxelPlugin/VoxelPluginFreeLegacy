// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "IntBox.h"

FORCEINLINE uint64 IntPow9(int Power)
{
	check(0 <= Power && Power <= 19);
	uint64 Pow = 1;
	for (int i = 0; i < Power; i++)
	{
		Pow *= 9;
	}
	return Pow;
}

template<class ElementType, uint32 ChunkSize>
class TVoxelOctree
{
public:
	// Center of the octree
	const FIntVector Position;

	// Distance to the highest resolution
	const uint8 LOD;

	// Id of the Octree (position in the octree)
	const uint64 Id;

	// Cached value
	const FIntBox Bounds;
	

	TVoxelOctree(uint8 LOD)
		: Position(FIntVector::ZeroValue)
		, LOD(LOD)
		, Id(IntPow9(LOD))
		, bIsLeaf(true)
		, Bounds(GetMinimalCornerPosition(), GetMaximalCornerPosition())
	{
		// Max for Id
		check(LOD <= 19);
	}
	virtual ~TVoxelOctree()
	{
		for (auto Child : Childs)
		{
			delete Child;
		}
	}

	/**
	 * Get the width at this level
	 * @return	Width of this chunk
	 */
	FORCEINLINE int Size() const
	{
		return ChunkSize << LOD;
	}

	FORCEINLINE FIntVector GetMinimalCornerPosition() const
	{
		return Position - FIntVector(Size() / 2, Size() / 2, Size() / 2);
	}
	FORCEINLINE FIntVector GetMaximalCornerPosition() const
	{
		return Position + FIntVector(Size() / 2, Size() / 2, Size() / 2);
	}

	FORCEINLINE FIntBox GetBounds() const
	{
		return Bounds;
	}

	/**
	 * Is Leaf?
	 * @return IsLeaf
	 */
	FORCEINLINE bool IsLeaf() const
	{
		return bIsLeaf;
	}

	/**
	 * Is the given Id a child of this octree?
	 */
	FORCEINLINE bool IsIdChild(uint64 ChildId) const
	{
		uint64 Pow = IntPow9(LOD);
		return Id / Pow == ChildId / Pow;
	}

	/**
	 * Is GlobalPosition in this octree?
	 * @param	GlobalPosition	Position in voxel space
	 * @return	If IsInOctree
	 */
	FORCEINLINE bool IsInOctree(int X, int Y, int Z) const
	{
		return Position.X - Size() / 2 <= X && X < Position.X + Size() / 2
			&& Position.Y - Size() / 2 <= Y && Y < Position.Y + Size() / 2
			&& Position.Z - Size() / 2 <= Z && Z < Position.Z + Size() / 2;
	}
	FORCEINLINE bool IsInOctree(const FIntVector& P) const
	{
		return IsInOctree(P.X, P.Y, P.Z);
	}

	/**
	 * Convert from chunk space to voxel space
	 * @param	LocalPosition	Position in chunk space
	 * @return	Position in voxel space
	 */
	FORCEINLINE void LocalToGlobal(int X, int Y, int Z, int& OutX, int& OutY, int& OutZ) const
	{
		OutX = X + (Position.X - Size() / 2);
		OutY = Y + (Position.Y - Size() / 2);
		OutZ = Z + (Position.Z - Size() / 2);
	}

	/**
	 * Convert from voxel space to chunk space
	 * @param	GlobalPosition	Position in voxel space
	 * @return	Position in chunk space
	 */
	FORCEINLINE void GlobalToLocal(int X, int Y, int Z, int& OutX, int& OutY, int& OutZ) const
	{
		OutX = X - (Position.X - Size() / 2);
		OutY = Y - (Position.Y - Size() / 2);
		OutZ = Z - (Position.Z - Size() / 2);
	}
	
	/**
	 * Get direct child that owns GlobalPosition
	 * @param	X,Y,Z	Global Position
	 */
	FORCEINLINE ElementType* GetChild(int X, int Y, int Z) const
	{
		check(!IsLeaf());
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		return Childs[(X >= Position.X) + 2 * (Y >= Position.Y) + 4 * (Z >= Position.Z)];
	}
	FORCEINLINE ElementType* GetChild(const FIntVector& P) const
	{
		return GetChild(P.X, P.Y, P.Z);
	}

	FORCEINLINE const TArray<ElementType*>& GetChilds() const
	{
		return this->Childs;
	}

	/**
	 * Get the octree leaf at global position
	 * @param	X,Y,Z	Global Position
	 */
	FORCEINLINE ElementType* GetLeaf(int X, int Y, int Z) const
	{
		check(IsInOctree(X, Y, Z));

		const ElementType* Ptr = static_cast<const ElementType*>(this);

		while (!Ptr->IsLeaf())
		{
			Ptr = Ptr->GetChild(X, Y, Z);
		}

		check(Ptr->IsInOctree(X, Y, Z));

		return const_cast<ElementType*>(Ptr);
	}
	FORCEINLINE ElementType* GetLeaf(const FIntVector& P) const
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
				for (auto Child : Childs)
				{
					Child->GetLeavesOverlappingBox(Box, OutOctrees);
				}
			}
		}
	}

	// Warning: OutOctrees is reversed
	void GetLeavesWithIds(TArray<uint64>& Ids, TArray<ElementType*>& OutOctrees)
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
					for (auto Child : GetChilds())
					{
						Child->GetLeavesWithIds(Ids, OutOctrees);
					}
				}
			}
		}
	}

protected:
	TVoxelOctree(FIntVector Position, uint8 LOD, uint64 Id)
		: Position(Position)
		, LOD(LOD)
		, Id(Id)
		, bIsLeaf(true)
		, Bounds(GetMinimalCornerPosition(), GetMaximalCornerPosition())
	{
		// Max for Id
		check(LOD <= 19);
	}

	TVoxelOctree(ElementType* Parent, uint8 ChildIndex)
		: Position(Parent->Position +
			FIntVector(
				Parent->Size() / 4 * ((ChildIndex & 1) ? 1 : -1),
				Parent->Size() / 4 * ((ChildIndex & 2) ? 1 : -1),
				Parent->Size() / 4 * ((ChildIndex & 4) ? 1 : -1)
			)
		)
		, LOD(Parent->LOD - 1)
		, Id(Parent->Id + IntPow9(Parent->LOD - 1) * (ChildIndex + 1))
		, bIsLeaf(true)
		, Bounds(GetMinimalCornerPosition(), GetMaximalCornerPosition())
	{
		check(0 <= ChildIndex && ChildIndex < 8);
	}

	/**
	 * Create childs of this octree
	 */
	virtual void CreateChilds()
	{		
		check(IsLeaf());
		check(Childs.Num() == 0);
		check(LOD != 0);

		int d = Size() / 4;
		uint64 Pow = IntPow9(LOD - 1);

		Childs.Add(new ElementType(static_cast<ElementType*>(this), 0));
		Childs.Add(new ElementType(static_cast<ElementType*>(this), 1));
		Childs.Add(new ElementType(static_cast<ElementType*>(this), 2));
		Childs.Add(new ElementType(static_cast<ElementType*>(this), 3));
		Childs.Add(new ElementType(static_cast<ElementType*>(this), 4));
		Childs.Add(new ElementType(static_cast<ElementType*>(this), 5));
		Childs.Add(new ElementType(static_cast<ElementType*>(this), 6));
		Childs.Add(new ElementType(static_cast<ElementType*>(this), 7));

		bIsLeaf = false;
	}

	virtual void DestroyChilds()
	{
		check(!IsLeaf());

		for (auto Child : Childs)
		{
			delete Child;
		}
		Childs.Reset();
		bIsLeaf = true;
	}


private:
	bool bIsLeaf;	
	/*
	Childs of this octree in the following order:

	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x
	*/
	TArray<ElementType*> Childs;
};