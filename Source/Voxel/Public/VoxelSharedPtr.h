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
	return TVoxelWeakPtr<T>(StaticCastSharedRef<T>(Ptr->AsShared()));
}

template<typename T, template<class, ESPMode> class TPtr>
inline TWeakPtr<T> MakeWeakPtr(const TPtr<T, ESPMode::Fast>& Ptr)
{
	return TWeakPtr<T>(Ptr);
}
template<typename T>
inline TWeakPtr<T> MakeWeakPtr(T* Ptr)
{
	return TWeakPtr<T>(StaticCastSharedRef<T>(Ptr->AsShared()));
}

// Need TEnableIf as &&& is equivalent to &, so T could get matched with Smthg&
template<typename T>
inline typename TEnableIf<!TIsReferenceType<T>::Value, TSharedRef<T>>::Type MakeSharedCopy(T&& Data)
{
	return MakeShared<T>(MoveTemp(Data));
}
template<typename T>
inline typename TEnableIf<!TIsReferenceType<T>::Value, TVoxelSharedRef<T>>::Type MakeVoxelSharedCopy(T&& Data)
{
	return MakeVoxelShared<T>(MoveTemp(Data));
}

template<typename T>
inline TSharedRef<T> MakeSharedCopy(const T& Data)
{
	return MakeShared<T>(Data);
}
template<typename T>
inline TVoxelSharedRef<T> MakeVoxelSharedCopy(const T& Data)
{
	return MakeVoxelShared<T>(Data);
}