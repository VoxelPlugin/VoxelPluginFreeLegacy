// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelOctree.h"

template<uint32 ChunkSize, class ElementType, typename Allocator = FMemory>
class TSimpleVoxelOctree
{
public:
	using ChildrenArray = ElementType[8];

	// Center of the octree
	const FIntVector Position;

	// Height of the octree (distance to smallest possible leaf)
	const uint8 Height;

	TSimpleVoxelOctree(uint8 Height)
		: Position(FIntVector::ZeroValue)
		, Height(Height)
	{
		check(Height < 32);
	}
	~TSimpleVoxelOctree()
	{
		if (HasChildren())
		{
			DestroyChildren();
		}
	}

public:
	inline uint32 Size() const
	{
		return ChunkSize << Height;
	}
	inline FVoxelIntBox GetBounds() const
	{
		return FVoxelIntBox(Position - Size() / 2, Position + Size() / 2);
	}
	inline bool HasChildren() const
	{
		return Children != nullptr;
	}
	inline FVoxelOctreeId GetId() const
	{
		return { Position, Height };
	}

public:
	inline const ElementType& GetChild(int32 X, int32 Y, int32 Z) const
	{
		return GetChild(GetChildIndex(X, Y, Z));
	}
	inline ElementType& GetChild(int32 X, int32 Y, int32 Z)
	{
		return GetChild(GetChildIndex(X, Y, Z));
	}

	inline const ElementType& GetChild(const FIntVector& P) const
	{
		return GetChild(P.X, P.Y, P.Z);
	}
	inline ElementType& GetChild(const FIntVector& P)
	{
		return GetChild(P.X, P.Y, P.Z);
	}

	inline const ElementType& GetChild(int32 Index) const
	{
		checkVoxelSlow((Children != nullptr) & (0 <= Index) & (Index < 8));
		return Children[Index];
	}
	inline ElementType& GetChild(int32 Index)
	{
		checkVoxelSlow((Children != nullptr) & (0 <= Index) & (Index < 8));
		return Children[Index];
	}

	inline const ChildrenArray& GetChildren() const
	{
		checkVoxelSlow(Children);
		return reinterpret_cast<const ChildrenArray&>(*Children);
	}
	inline ChildrenArray& GetChildren()
	{
		checkVoxelSlow(Children);
		return reinterpret_cast<ChildrenArray&>(*Children);
	}

protected:
	TSimpleVoxelOctree(const ElementType& Parent, uint8 ChildIndex)
		: Position(GetChildPosition(Parent.Position, Parent.Size(), ChildIndex))
		, Height(Parent.Height - 1)
	{
		checkVoxelSlow(0 <= ChildIndex && ChildIndex < 8);
	}

	template<typename... TArgs>
	inline void CreateChildren(TArgs&&... Args)
	{		
		check(!HasChildren() && Height > 0);

		Children = static_cast<ElementType*>(Allocator::Malloc(8 * sizeof(ElementType)));

		for (int32 Index = 0; Index < 8 ; Index++)
		{
			new (&Children[Index]) ElementType(This(), Index, Forward<TArgs>(Args)...);
		}
	}

	inline void DestroyChildren()
	{
		check(HasChildren());

		for (auto& Child : GetChildren())
		{
			Child.~ElementType();
		}

		Allocator::Free(Children);
		Children = nullptr;
	}


private:
	ElementType* Children = nullptr;

	inline int32 GetChildIndex(int32 X, int32 Y, int32 Z) const
	{
		return (X >= Position.X) + 2 * (Y >= Position.Y) + 4 * (Z >= Position.Z);
	}
	inline static FIntVector GetChildPosition(const FIntVector& ParentPosition, uint32 ParentSize, uint8 ChildIndex)
	{
		return ParentPosition +
			FIntVector(
				ParentSize / 4 * ((ChildIndex & 0x1) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x2) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x4) ? 1 : -1));
	}

	inline ElementType& This()
	{
		return static_cast<ElementType&>(*this);
	}
	inline const ElementType& This() const
	{
		return static_cast<const ElementType&>(*this);
	}
};