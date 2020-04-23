// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

// Base class for blueprint structs that hold a pointer to a big implementation struct that's expensive to copy around
template<typename T>
struct TVoxelSaveStruct
{
public:
	bool operator==(const TVoxelSaveStruct<T>& Other) const
	{
		return Impl == Other.Impl || *Impl == *Other.Impl;
	}
	bool Serialize(FArchive& Ar)
	{
		if (Ar.IsLoading())
		{
			// If we're going to write, create a new impl
			Impl = MakeShared<T>();
		}
		return Impl->Serialize(Ar);
	}
	const T& Const() const
	{
		return *Impl;
	}
	// Copies the data over from old impl
	T& Mutable()
	{
		if (Impl.IsUnique())
		{
			return *Impl;
		}
		else
		{
			Impl = MakeShared<T>(*Impl);
			return *Impl;
		}
	}
	// Does not copy over the old impl data
	T& NewMutable()
	{
		Impl = MakeShared<T>();
		return *Impl;
	}

private:
	TSharedRef<T> Impl = MakeShared<T>();
};

#define DEFINE_VOXEL_SAVE_STRUCT(Name) \
	inline FArchive& operator<<(FArchive& Ar, Name& Save) \
	{ \
		Save.Serialize(Ar); \
		return Ar; \
	} \
	template<> \
	struct TStructOpsTypeTraits<Name> : public TStructOpsTypeTraitsBase2<Name> \
	{ \
		enum  \
		{ \
			WithSerializer = true, \
			WithIdenticalViaEquality = true \
		}; \
	};