// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelOctreeId.h"

template<uint32 ChunkSize>
class TVoxelOctreeBase
{
public:
	// Height of the octree (distance to smallest possible leaf)
	const uint8 Height;

	// Center of the octree
	const FIntVector Position;

	TVoxelOctreeBase(uint8 Height, const FIntVector& Position)
		: Height(Height)
		, Position(Position)
	{
		check(Height < 32);
	}
	~TVoxelOctreeBase() = default;

public:
	FORCEINLINE uint32 GetSize() const
	{
		return ChunkSize << Height;
	}
	FORCEINLINE uint32 GetHalfSize() const
	{
		return (ChunkSize / 2) << Height;
	}
	FORCEINLINE FIntVector GetMin() const
	{
		return Position - GetHalfSize();
	}
	FORCEINLINE FIntVector GetMax() const
	{
		return Position + GetHalfSize();
	}
	FORCEINLINE FVoxelIntBox GetBounds() const
	{
		return FVoxelIntBox(GetMin(), GetMax());
	}
	FORCEINLINE bool IsInOctree(int32 X, int32 Y, int32 Z) const
	{
		const int32 HalfSize = GetHalfSize();
		return
			Position.X - HalfSize <= X && X < Position.X + HalfSize &&
			Position.Y - HalfSize <= Y && Y < Position.Y + HalfSize &&
			Position.Z - HalfSize <= Z && Z < Position.Z + HalfSize;
	}
	FORCEINLINE bool IsInOctree(const FIntVector& P) const
	{
		return IsInOctree(P.X, P.Y, P.Z);
	}
	FORCEINLINE bool IsLeaf() const
	{
		return Height == 0;
	}
	FORCEINLINE FVoxelOctreeId GetId() const
	{
		return { Position, Height };
	}

protected:
	FORCEINLINE static FIntVector GetChildPosition(const FIntVector& ParentPosition, uint32 ParentSize, uint8 ChildIndex)
	{
		return ParentPosition +
			FIntVector(
				ParentSize / 4 * ((ChildIndex & 0x1) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x2) ? 1 : -1),
				ParentSize / 4 * ((ChildIndex & 0x4) ? 1 : -1));
	}
};

template<typename BaseType>
class TVoxelOctreeLeaf : public BaseType
{
protected:
	TVoxelOctreeLeaf(const BaseType& Parent, uint8 ChildIndex)
		: BaseType(Parent.Height - 1, this->GetChildPosition(Parent.Position, Parent.GetSize(), ChildIndex))
	{
		check(0 <= ChildIndex && ChildIndex < 8);
	}
};

template<typename BaseType, typename LeafType, typename ParentType>
class TVoxelOctreeParent : public BaseType
{
public:
	template<typename T>
	struct FChildrenIterator
	{
		void* const Children;
		bool const bIsLeaf;

		FChildrenIterator(void* Children, bool bIsLeaf)
			: Children(Children)
			, bIsLeaf(bIsLeaf)
		{
		}

		struct FIterator
		{
			void* const VoidPtr;
			bool const bIsLeaf;
			uint32 Index = 0;

			FIterator(void* Ptr, bool bIsLeaf)
				: VoidPtr(Ptr)
				, bIsLeaf(bIsLeaf)
			{
			}

			inline T& operator*() const
			{
				if (bIsLeaf)
				{
					LeafType* Ptr = reinterpret_cast<LeafType*>(VoidPtr);
					return static_cast<T&>(Ptr[Index]);
				}
				else
				{
					ParentType* Ptr = reinterpret_cast<ParentType*>(VoidPtr);
					return static_cast<T&>(Ptr[Index]);
				}
			}
			inline void operator++()
			{
				Index++;
			}
			inline bool operator!=(const FIterator&) const
			{
				return Index != 8;
			}
		};
		
		FIterator begin() { return FIterator(Children, bIsLeaf); }
		FIterator end() { return FIterator(nullptr, false); }
	};

public:
	TVoxelOctreeParent(uint8 Height)
		: BaseType(Height, FIntVector(0, 0, 0))
	{
	}
	~TVoxelOctreeParent()
	{
		if (HasChildren())
		{
			DestroyChildren();
		}
	}

public:
	inline bool HasChildren() const
	{
		return Children != nullptr;
	}

public:
	inline const BaseType& GetChild(int32 X, int32 Y, int32 Z) const
	{
		return GetChild(GetChildIndex(X, Y, Z));
	}
	inline BaseType& GetChild(int32 X, int32 Y, int32 Z)
	{
		return GetChild(GetChildIndex(X, Y, Z));
	}
	
	inline const BaseType& GetChild(const FIntVector& P) const
	{
		return GetChild(P.X, P.Y, P.Z);
	}
	inline BaseType& GetChild(const FIntVector& P)
	{
		return GetChild(P.X, P.Y, P.Z);
	}

	inline const BaseType& GetChild(int32 Index) const
	{
		check((Children != nullptr) & (0 <= Index) & (Index < 8));
		if (this->Height == 1)
		{
			const LeafType* Ptr = reinterpret_cast<const LeafType*>(Children);
			return static_cast<const BaseType&>(Ptr[Index]);
		}
		else
		{
			const ParentType* Ptr = reinterpret_cast<const ParentType*>(Children);
			return static_cast<const BaseType&>(Ptr[Index]);
		}
	}
	inline BaseType& GetChild(int32 Index)
	{
		check((Children != nullptr) & (0 <= Index) & (Index < 8));
		if (this->Height == 1)
		{
			LeafType* Ptr = reinterpret_cast<LeafType*>(Children);
			return static_cast<BaseType&>(Ptr[Index]);
		}
		else
		{
			ParentType* Ptr = reinterpret_cast<ParentType*>(Children);
			return static_cast<BaseType&>(Ptr[Index]);
		}
	}

	inline FChildrenIterator<const BaseType> GetChildren() const
	{
		check(Children);
		return { Children, this->Height == 1 };
	}
	inline FChildrenIterator<BaseType> GetChildren()
	{
		check(Children);
		return { Children, this->Height == 1 };
	}

protected:
	TVoxelOctreeParent(const TVoxelOctreeParent& Parent, uint8 ChildIndex)
		: BaseType(Parent.Height - 1, this->GetChildPosition(Parent.Position, Parent.GetSize(), ChildIndex))
	{
		check(0 <= ChildIndex && ChildIndex < 8);
	}

	template<typename... TArgs>
	inline void CreateChildren(TArgs&&... Args)
	{		
		check(!HasChildren() && this->Height > 0);

		// TODO PERF: Specialized malloc call
		Children = FMemory::Malloc(8 * (this->Height == 1 ? sizeof(LeafType) : sizeof(ParentType)));

		for (int32 Index = 0; Index < 8 ; Index++)
		{
			if (this->Height == 1)
			{
				LeafType* Ptr = static_cast<LeafType*>(Children);
				new (&Ptr[Index]) LeafType  (static_cast<const ParentType&>(*this), Index, Forward<TArgs>(Args)...);
			}
			else
			{
				ParentType* Ptr = static_cast<ParentType*>(Children);
				new (&Ptr[Index]) ParentType(static_cast<const ParentType&>(*this), Index, Forward<TArgs>(Args)...);
			}
		}
	}
	inline void DestroyChildren()
	{
		check(HasChildren());

		for (auto& Child : GetChildren())
		{
			if (this->Height == 1)
			{
				static_cast<LeafType&>(Child).~LeafType();
			}
			else
			{
				static_cast<ParentType&>(Child).~ParentType();
			}
		}

		FMemory::Free(Children);
		Children = nullptr;
	}

private:
	void* Children = nullptr;

	inline uint32 GetChildIndex(int32 X, int32 Y, int32 Z) const
	{
		return (X >= this->Position.X) + 2 * (Y >= this->Position.Y) + 4 * (Z >= this->Position.Z);
	}
};