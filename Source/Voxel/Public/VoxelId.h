// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"

#define MAX_WORLD_DEPTH_HALF (MAX_WORLD_DEPTH / 2)

// First bit: End?
// Second bit: X > 0?
// Third bit: Y > 0?
// Fourth bit: Z > 0?

struct FVoxelId
{
	uint8 B[MAX_WORLD_DEPTH_HALF];

	FVoxelId()
	{
	}

	FVoxelId(EForceInit Init)
	{
		if (Init == EForceInit::ForceInitToZero)
		{
			for (int32 I = 0; I < MAX_WORLD_DEPTH_HALF; I++)
			{
				B[I] = 0;
			}
		}
	}

	inline static FVoxelId FromParent(const FVoxelId& ParentId, uint8 ChildLOD, uint8 ChildIndex)
	{
		check(ChildIndex < 16);
		check(ChildLOD < MAX_WORLD_DEPTH);
		FVoxelId R = ParentId;
		R.B[ChildLOD / 2] |= ChildIndex << ((ChildLOD % 2) * 4);
		return R;
	}

	inline static bool IsChild(const FVoxelId& Parent, const FVoxelId& Child)
	{
		for (int32 I = 0; I < MAX_WORLD_DEPTH_HALF; I++)
		{
			if (((Parent.B[I] & 0x0F) != 0 && ((Parent.B[I] & 0x0F) != (Child.B[I] & 0x0F))) || 
				((Parent.B[I] & 0xF0) != 0 && ((Parent.B[I] & 0xF0) != (Child.B[I] & 0xF0))))
			{
				return false;
			}
		}
		return true;
	}

	inline bool operator==(const FVoxelId& Other) const
	{
		for (int32 I = 0; I < MAX_WORLD_DEPTH_HALF; I++)
		{
			if (B[I] != Other.B[I])
			{
				return false;
			}
		}
		return true;
	}

	inline bool operator!=(const FVoxelId& Other) const
	{
		for (int32 I = 0; I < MAX_WORLD_DEPTH_HALF; I++)
		{
			if (B[I] != Other.B[I])
			{
				return true;
			}
		}
		return false;
	}
};

inline FArchive& operator<<(FArchive &Ar, FVoxelId& Id)
{
	for (int32 I = 0; I < MAX_WORLD_DEPTH_HALF; I++)
	{
		Ar << Id.B[I];
	}

	return Ar;
}