// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Async/Async.h"
#include "VoxelUtilities/VoxelSystemUtilities.h"

namespace FVoxelUtilities
{
	// Call this when you pin a shared ptr on another thread that needs to always be deleted on the game thread
	template<typename T>
	void RunOnGameThread(T&& Lambda)
	{
		if (IsInGameThread())
		{
			Lambda();
		}
		else
		{
			check(FTaskGraphInterface::IsRunning());
			AsyncTask(ENamedThreads::GameThread, MoveTemp(Lambda));
		}
	}
	
	template<typename T>
	void DeleteTickable(T* Ptr)
	{
		// There is a bug in 4.23/24 where FTickableGameObject gets added to a set of deleted tickable objects on destruction
		// This set is then checked in the next frame before adding a new tickable to see if it has been deleted
		// See Engine/Source/Runtime/Engine/Private/Tickable.cpp:107
		// The problem is that when deleting a tickable, there is a large chance than if we create another tickable of the same class
		// it'll get assigned the same ptr (as the memory allocator will have a request of the exact same size, so will reuse freshly deleted ptr)
		// This set of ptr is only valid one frame. To bypass this bug, we are postponing the tickable deletion for 1s
		// Fixed by https://github.com/EpicGames/UnrealEngine/commit/70d70e56f2df9ba6941b91d9893ba6c6e99efc4c
		if (ENGINE_MINOR_VERSION < 25)
		{
			FVoxelUtilities::DelayedCall([=]()
			{
				delete Ptr;
			}, 1.f);
		}
		else
		{
			delete Ptr;
		}
	}

	template<typename T, bool bIsTickable>
	struct TGameThreadDeleter
	{
		void operator()(T* Object) const
		{
			if (!Object) return;
			
			FVoxelUtilities::RunOnGameThread([=]()
			{
				if (bIsTickable)
				{
					FVoxelUtilities::DeleteTickable(Object);
				}
				else
				{
					delete Object;
				}
			});
		}
	};

	template<typename T,  typename... TArgs>
	static TVoxelSharedRef<T> MakeGameThreadDeleterPtr(TArgs&&... Args)
	{
		return TVoxelSharedPtr<T>(new T(Forward<TArgs>(Args)...), TGameThreadDeleter<T, false>()).ToSharedRef();
	}
	template<typename T,  typename... TArgs>
	static TVoxelSharedRef<T> MakeGameThreadTickableDeleterPtr(TArgs&&... Args)
	{
		return TVoxelSharedPtr<T>(new T(Forward<TArgs>(Args)...), TGameThreadDeleter<T, true>()).ToSharedRef();
	}

	template<typename TGetPerThreadData, typename TLambda>
	void ParallelFor_PerThreadData(int32 Num, TGetPerThreadData GetPerThreadData, TLambda Lambda, bool bForceSingleThread = false)
	{
		if (Num == 0 || !ensure(Num > 0))
		{
			return;
		}
		
		if (bForceSingleThread)
		{
			auto PerThreadData = GetPerThreadData();
			for (int32 Index = 0; Index < Num; Index++)
			{
				Lambda(PerThreadData, Index);
			}
		}
		else
		{
			const int32 NumThreads = FMath::Min<int32>(FTaskGraphInterface::Get().GetNumWorkerThreads(), Num);
			ensure(NumThreads < 64); // Else bad perf below

			using TData = typename TDecay<decltype(GetPerThreadData())>::Type;
			TArray<TData, TInlineAllocator<64>> PerThreadDataArray;
			PerThreadDataArray.Reserve(NumThreads);
			for (int32 Index = 0; Index < NumThreads; Index++)
			{
				PerThreadDataArray.Emplace(GetPerThreadData());
			}

			const int32 ChunkSize = Num / NumThreads;
			check(ChunkSize >= 1);

			ParallelFor(NumThreads, [&](int32 ThreadIndex)
			{
				auto& ThreadData = PerThreadDataArray[ThreadIndex];
				
				const int32 Start = ThreadIndex * ChunkSize;
				const int32 End = FMath::Min((ThreadIndex + 1) * ChunkSize, Num);
				for (int32 Index = Start; Index < End; Index++)
				{
					Lambda(ThreadData, Index);
				}
			});
		}
		
	}
}