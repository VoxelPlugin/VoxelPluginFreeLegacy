// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelOctree.h"

template<class ElementType>
class TSimpleVoxelOctree
{
public:
	using ChildrenArray = ElementType[8];

	const uint32 ChunkSize;
	
	// Center of the octree
	const FIntVector Position;

	// Height of the octree (distance to smallest possible leaf)
	const uint8 Height;

	TSimpleVoxelOctree(uint32 ChunkSize, uint8 Height)
		: ChunkSize(ChunkSize)
		, Position(FIntVector::ZeroValue)
		, Height(Height)
	{
		check(Height < 32);
		check(FMath::IsPowerOfTwo(ChunkSize));
	}
	~TSimpleVoxelOctree()
	{
		if (HasChildren())
		{
			DestroyChildren();
		}
	}

public:
	uint32 Size() const
	{
		return ChunkSize << Height;
	}
	FVoxelIntBox GetBounds() const
	{
		return FVoxelIntBox(Position - Size() / 2, Position + Size() / 2);
	}
	bool HasChildren() const
	{
		return Children != nullptr;
	}
	FVoxelOctreeId GetId() const
	{
		return { Position, Height };
	}

public:
	const ElementType& GetChild(int32 X, int32 Y, int32 Z) const
	{
		return GetChild(GetChildIndex(X, Y, Z));
	}
	ElementType& GetChild(int32 X, int32 Y, int32 Z)
	{
		return GetChild(GetChildIndex(X, Y, Z));
	}

	const ElementType& GetChild(const FIntVector& P) const
	{
		return GetChild(P.X, P.Y, P.Z);
	}
	ElementType& GetChild(const FIntVector& P)
	{
		return GetChild(P.X, P.Y, P.Z);
	}

	const ElementType& GetChild(int32 Index) const
	{
		checkVoxelSlow((Children != nullptr) & (0 <= Index) & (Index < 8));
		return Children[Index];
	}
	ElementType& GetChild(int32 Index)
	{
		checkVoxelSlow((Children != nullptr) & (0 <= Index) & (Index < 8));
		return Children[Index];
	}

	const ChildrenArray& GetChildren() const
	{
		checkVoxelSlow(Children);
		return reinterpret_cast<const ChildrenArray&>(*Children);
	}
	ChildrenArray& GetChildren()
	{
		checkVoxelSlow(Children);
		return reinterpret_cast<ChildrenArray&>(*Children);
	}

protected:
	TSimpleVoxelOctree(const ElementType& Parent, uint8 ChildIndex)
		: ChunkSize(Parent.ChunkSize)
		, Position(GetChildPosition(Parent.Position, Parent.Size(), ChildIndex))
		, Height(Parent.Height - 1)
	{
		checkVoxelSlow(0 <= ChildIndex && ChildIndex < 8);
	}

	template<typename... TArgs>
	void CreateChildren(TArgs&&... Args)
	{		
		check(!HasChildren() && Height > 0);

		Children = static_cast<ElementType*>(FMemory::Malloc(8 * sizeof(ElementType)));

		for (int32 Index = 0; Index < 8 ; Index++)
		{
			new (&Children[Index]) ElementType(This(), Index, Forward<TArgs>(Args)...);
		}
	}

	void DestroyChildren()
	{
		check(HasChildren());

		for (auto& Child : GetChildren())
		{
			Child.~ElementType();
		}

		FMemory::Free(Children);
		Children = nullptr;
	}


private:
	ElementType* Children = nullptr;

	int32 GetChildIndex(int32 X, int32 Y, int32 Z) const
	{
		return (X >= Position.X) + 2 * (Y >= Position.Y) + 4 * (Z >= Position.Z);
	}
	static FIntVector GetChildPosition(const FIntVector& ParentPosition, uint32 ParentSize, uint8 ChildIndex)
	{
		return ParentPosition +
			FIntVector(
				ParentSize / 4 * ((ChildIndex & 0x1) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x2) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x4) ? 1 : -1));
	}

	ElementType& This()
	{
		return static_cast<ElementType&>(*this);
	}
	const ElementType& This() const
	{
		return static_cast<const ElementType&>(*this);
	}
};