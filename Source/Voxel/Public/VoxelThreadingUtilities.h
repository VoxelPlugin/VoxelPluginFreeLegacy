// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "Async/Async.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
	template<typename T>
	inline void DeleteTickable(UWorld* World, TVoxelSharedPtr<T>& Ptr)
	{
		// There is a bug in 4.23/24 where FTickableGameObject gets added to a set of deleted tickable objects on destruction
		// This set is then checked in the next frame before adding a new tickable to see if it has been deleted
		// See Engine/Source/Runtime/Engine/Private/Tickable.cpp:107
		// The problem is that when deleting a tickable, there is a large chance than if we create another tickable of the same class
		// it'll get assigned the same ptr (as the memory allocator will have a request of the exact same size, so will reuse freshly deleted ptr)
		// This set of ptr is only valid one frame. To bypass this bug, we are postponing the tickable deletion for 1s
		// Fixed by https://github.com/EpicGames/UnrealEngine/commit/70d70e56f2df9ba6941b91d9893ba6c6e99efc4c
		ensure(Ptr.IsValid());
		if (World)
		{
			// No world when exiting
			FTimerManager& TimerManager = World->GetTimerManager();
			FTimerHandle Handle;
			TimerManager.SetTimer(
				Handle,
				FTimerDelegate::CreateLambda([PtrPtr = MakeVoxelShared<TVoxelSharedPtr<T>>(Ptr)]() { ensure(PtrPtr->IsValid()); PtrPtr->Reset(); }),
				1.f,
				false);
			ensure(!Ptr.IsUnique());
		}
		Ptr.Reset();
	}
}