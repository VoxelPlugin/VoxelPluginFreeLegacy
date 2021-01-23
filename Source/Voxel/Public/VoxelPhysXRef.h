// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

#if WITH_PHYSX && PHYSICS_INTERFACE_PHYSX
template<typename T>
class TVoxelPhysXRef
{
public:
	TVoxelPhysXRef() = default;
	TVoxelPhysXRef(T* Ptr)
	{
		if (Ptr)
		{
			Impl = MakeVoxelShared<FImpl>(*Ptr);
		}
	}

	T* Get() const { return Impl ? &Impl->Ptr : nullptr; }

private:
	struct FImpl
	{
		T& Ptr;
		
		FImpl(T& Ptr)
			: Ptr(Ptr)
		{
			Ptr.acquireReference();
		}
		~FImpl()
		{
			Ptr.release();
		}
	};
	TVoxelSharedPtr<FImpl> Impl;
};
#endif