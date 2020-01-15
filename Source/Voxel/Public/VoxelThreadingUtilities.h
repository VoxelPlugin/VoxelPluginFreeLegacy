// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "Async/Async.h"

namespace FVoxelUtilities
{
	// Call this when you pin a shared ptr on another thread that needs to always be deleted on the game thread
	template<typename T>
	inline void DeleteOnGameThread_AnyThread(TVoxelSharedPtr<T>& Ptr)
	{
		if (!ensure(!IsInGameThread()))
		{
			Ptr.Reset();
			return;
		}
		if (!ensure(Ptr.IsValid()))
		{
			return;
		}

		check(FTaskGraphInterface::IsRunning());

		// Always start a task to avoid race conditions
		AsyncTask(ENamedThreads::GameThread, [Ptr = MoveTemp(Ptr)]() { ensure(Ptr.IsValid()); });

		check(!Ptr.IsValid());
	}
	
	template<typename... TArgs, typename T, typename TLambda>
	inline auto MakeVoxelWeakPtrLambda(const T& Ptr, TLambda Lambda)
	{
		return [WeakPtr = MakeVoxelWeakPtr(Ptr), Lambda](TArgs... Args)
		{
			auto Pinned = WeakPtr.Pin();
			if (Pinned.IsValid())
			{
				Lambda(*Pinned, Forward<TArgs>(Args)...);
			}
		};
	}
	template<typename RetVal = void, typename... TArgs, typename T, typename TLambda>
	inline auto MakeVoxelWeakPtrDelegate(const T& Ptr, TLambda Lambda)
	{
		return TBaseDelegate<RetVal, TArgs...>::CreateLambda(MakeVoxelWeakPtrLambda<TArgs...>(Ptr, Lambda));
	}
	template<typename... TArgs, typename T, typename TLambda>
	inline auto MakeVoxelWeakPtrDelegate_GameThreadDelete(const T& Ptr, TLambda Lambda)
	{
		return [WeakPtr = MakeVoxelWeakPtr(Ptr), Lambda](TArgs... Args)
		{
			auto Pinned = WeakPtr.Pin();
			if (Pinned.IsValid())
			{
				Lambda(*Pinned, Forward<TArgs>(Args)...);
				DeleteOnGameThread_AnyThread(Pinned);
			}
		};
	}
}