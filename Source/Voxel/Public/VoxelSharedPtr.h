// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

template<typename T>
using TVoxelSharedRef = TSharedRef<T, ESPMode::ThreadSafe>;
template<typename T>
using TVoxelSharedPtr = TSharedPtr<T, ESPMode::ThreadSafe>;
template<typename T>
using TVoxelWeakPtr = TWeakPtr<T, ESPMode::ThreadSafe>;
template<typename T>
using TVoxelSharedFromThis = TSharedFromThis<T, ESPMode::ThreadSafe>;

template<typename CastToType, typename CastFromType>
inline TVoxelSharedRef<CastToType> StaticCastVoxelSharedRef(const TVoxelSharedRef<CastFromType>& InSharedRef)
{
	return StaticCastSharedRef<CastToType, CastFromType, ESPMode::ThreadSafe>(InSharedRef);
}

template<typename CastToType, typename CastFromType>
inline TVoxelSharedPtr<CastToType> StaticCastVoxelSharedPtr(const TVoxelSharedPtr<CastFromType>& InSharedPtr)
{
	return StaticCastSharedPtr<CastToType, CastFromType, ESPMode::ThreadSafe>(InSharedPtr);
}

template<typename T, typename... InArgTypes>
inline TVoxelSharedRef<T> MakeVoxelShared(InArgTypes&&... Args)
{
	return MakeShared<T, ESPMode::ThreadSafe>(Forward<InArgTypes>(Args)...);
}

template<typename T, template<class, ESPMode> class TPtr>
inline TVoxelWeakPtr<T> MakeVoxelWeakPtr(const TPtr<T, ESPMode::ThreadSafe>& Ptr)
{
	return TVoxelWeakPtr<T>(Ptr);
}
template<typename T>
inline TVoxelWeakPtr<T> MakeVoxelWeakPtr(T* Ptr)
{
	return TVoxelWeakPtr<T>(StaticCastVoxelSharedRef<T>(Ptr->AsShared()));
}