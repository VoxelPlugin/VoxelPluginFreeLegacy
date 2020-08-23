// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Containers/SparseArray.h"

// Default one does not do check on element access
template<typename InElementType, typename InAllocator = FDefaultSparseArrayAllocator>
class TVoxelSparseArray : public TSparseArray<InElementType, InAllocator>
{
public:
	using Super = TSparseArray<InElementType, InAllocator>;
	
	// Accessors.
	inline InElementType& operator[](int32 Index)
	{
		check(Index >= 0 && Index < this->GetMaxIndex() && this->IsValidIndex(Index));
		return Super::operator[](Index);
	}
	inline const InElementType& operator[](int32 Index) const
	{
		check(Index >= 0 && Index < this->GetMaxIndex() && this->IsValidIndex(Index));
		return Super::operator[](Index);
	}
};

// UniqueClass: to forbid copying ids from different classes
template<typename UniqueClass>
class TVoxelTypedSparseArrayId
{
public:
	TVoxelTypedSparseArrayId() = default;

	bool operator==(TVoxelTypedSparseArrayId Other) const { return Other.Index == Index; }
	bool operator!=(TVoxelTypedSparseArrayId Other) const { return Other.Index != Index; }

public:
	bool IsValid() const
	{
		return Index != 0;
	}
	void Reset()
	{
		Index = 0;
	}
	uint32 GetDebugValue() const
	{
		return Index;
	}

public:
	friend uint32 GetTypeHash(TVoxelTypedSparseArrayId Value)
	{
		return GetTypeHash(Value.Index);
	}
	
private:
	TVoxelTypedSparseArrayId(uint32 Index) : Index(Index) {}

	uint32 Index = 0;

	template<typename InKeyType, typename InElementType, typename InAllocator>
	friend class TVoxelTypedSparseArray;
};

#define DEFINE_TYPED_VOXEL_SPARSE_ARRAY_ID(Name) using Name = TVoxelTypedSparseArrayId<class Name##_Unique>;

template<typename InKeyType, typename InElementType, typename InAllocator = FDefaultSparseArrayAllocator>
class TVoxelTypedSparseArray
{
public:
	inline InElementType& operator[](InKeyType Index)
	{
		check(Index.IsValid());
		return Storage[Index.Index - 1];
	}
	inline const InElementType& operator[](InKeyType Index) const
	{
		check(Index.IsValid());
		return Storage[Index.Index - 1];
	}
	inline bool IsValidIndex(InKeyType Index) const
	{
		return Index.IsValid() && Storage.IsValidIndex(Index.Index - 1);
	}
	inline InKeyType Add(const InElementType& Element)
	{
		return { uint32(Storage.Add(Element)) + 1 };
	}
	inline InKeyType Add(InElementType&& Element)
	{
		return { uint32(Storage.Add(MoveTemp(Element))) + 1 };
	}
	inline void RemoveAt(InKeyType Index)
	{
		check(Index.IsValid());
		Storage.RemoveAt(Index.Index - 1);
	}
	inline int32 Num() const
	{
		return Storage.Num();
	}
	
public:
	inline auto begin()       { return Storage.begin(); }
	inline auto begin() const { return Storage.begin(); }
	inline auto end  ()       { return Storage.end(); }
	inline auto end  () const { return Storage.end(); }

private:
	TVoxelSparseArray<InElementType, InAllocator> Storage;
};