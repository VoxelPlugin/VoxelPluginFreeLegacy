// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "Misc/ScopeLock.h"
#include <mutex>
#include <condition_variable>

enum class EVoxelLockType
{
	Read,
	Write
};

class FVoxelSharedMutex
{
public:
	void Lock(EVoxelLockType LockType)
	{
#if DO_THREADSAFE_CHECKS
		AddThreadId();
#endif
		if (LockType == EVoxelLockType::Read)
		{
			std::unique_lock<std::mutex> Lock(Mutex);
			while (bWriting)
			{
				WriteQueue.wait(Lock);
			}
			NumReaders++;
		}
		else
		{
			std::unique_lock<std::mutex> Lock(Mutex);
			while (bWriting)
			{
				WriteQueue.wait(Lock);
			}

			bWriting = true;

			while (0 < NumReaders)
			{
				ReadQueue.wait(Lock);
			}
		}
	}
	void Unlock(EVoxelLockType LockType)
	{
#if DO_THREADSAFE_CHECKS
		RemoveThreadId();
#endif
		if (LockType == EVoxelLockType::Read)
		{
			uint32 NumReaders_Local;
			bool bWriting_Local;
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				NumReaders--;
				checkf(NumReaders >= 0, TEXT("Unlock Read called, but not locked for read!"));
				NumReaders_Local = NumReaders;
				bWriting_Local = bWriting;
			}

			if (bWriting_Local && NumReaders_Local == 0)
			{
				ReadQueue.notify_one();
			}
		}
		else
		{
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				checkf(bWriting, TEXT("Unlock Write called, but not locked for write!"));
				bWriting = false;
			}

			WriteQueue.notify_all();
		}
	}

	FORCEINLINE bool IsLockedForRead() const
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		return bWriting || NumReaders > 0;
	}
	FORCEINLINE bool IsLockedForWrite() const
	{
		std::lock_guard<std::mutex> Lock(Mutex);
		return bWriting;
	}
	
private:
	mutable std::mutex Mutex;
	std::condition_variable ReadQueue;
	std::condition_variable WriteQueue;
	int32 NumReaders = 0;
	bool bWriting = false;

#if DO_THREADSAFE_CHECKS
	FCriticalSection ThreadIdsSection;
	TArray<uint32, TInlineAllocator<16>> ThreadIds;

	void AddThreadId()
	{
		const uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		FScopeLock ScopeLock(&ThreadIdsSection);
		checkf(!ThreadIds.Contains(ThreadId), TEXT("Mutex already locked by this thread!"));
		ThreadIds.Add(ThreadId);
	}
	void RemoveThreadId()
	{
		const uint32 ThreadId = FPlatformTLS::GetCurrentThreadId();
		FScopeLock ScopeLock(&ThreadIdsSection);
		checkf(ThreadIds.Contains(ThreadId), TEXT("Mutex not locked by this thread!"));
		verify(ThreadIds.RemoveSwap(ThreadId) == 1);
	}
#endif
};