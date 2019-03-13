// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include <shared_mutex>

enum EVoxelLockType
{
	Read,
	ReadWrite
};

class FVoxelSharedMutex
{
public:
	template<EVoxelLockType>
	void Lock();
	template<EVoxelLockType>
	void Unlock();
	
	template<EVoxelLockType>
	bool TryLockUntil(double TimeToTimeout);
	
private:
	std::shared_timed_mutex Mutex;
	
	template<EVoxelLockType>
	bool TryLockImpl();

#if DO_THREADSAFE_CHECKS
public:
	inline bool IsLockedForRead() const
	{
		if (ReadLocks.GetValue())
		{
			return true;
		}
		else
		{
			return WriteLocks.GetValue();
		}
	}
	inline bool IsLockedForWrite() const
	{
		if (WriteLocks.GetValue())
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	template<EVoxelLockType LockType>
	inline bool IsLocked() const
	{
		return LockType == Read ? IsLockedForRead() : IsLockedForWrite();
	}

private:
	FThreadSafeCounter ReadLocks;
	FThreadSafeCounter WriteLocks;
#endif
};

template<> inline void FVoxelSharedMutex::Lock<EVoxelLockType::Read>()
{
	Mutex.lock_shared();
#if DO_THREADSAFE_CHECKS
	ReadLocks.Increment();
#endif
}
template<> inline void FVoxelSharedMutex::Lock<EVoxelLockType::ReadWrite>()
{
	Mutex.lock();
#if DO_THREADSAFE_CHECKS
	WriteLocks.Increment();
#endif
}

template<> inline void FVoxelSharedMutex::Unlock<EVoxelLockType::Read>()
{
	Mutex.unlock_shared();
#if DO_THREADSAFE_CHECKS
	ReadLocks.Decrement();
	ensure(ReadLocks.GetValue() >= 0);
#endif
}
template<> inline void FVoxelSharedMutex::Unlock<EVoxelLockType::ReadWrite>()
{
	Mutex.unlock();
#if DO_THREADSAFE_CHECKS
	WriteLocks.Decrement();
	ensure(WriteLocks.GetValue() >= 0);
#endif
}

template<> inline bool FVoxelSharedMutex::TryLockImpl<EVoxelLockType::Read>()
{
	if (Mutex.try_lock_shared())
	{
#if DO_THREADSAFE_CHECKS
		ReadLocks.Increment();
#endif
		return true;
	}
	else
	{
		return false;
	}
}
template<> inline bool FVoxelSharedMutex::TryLockImpl<EVoxelLockType::ReadWrite>()
{
	if (Mutex.try_lock())
	{
#if DO_THREADSAFE_CHECKS
		WriteLocks.Increment();
#endif
		return true;
	}
	else
	{
		return false;
	}
}

// The try_lock_for method of std::shared_time_mutex isn't precise enough for us, so we do a bit of active waiting
template<EVoxelLockType LockType>
bool inline FVoxelSharedMutex::TryLockUntil(double TimeToTimeout)
{
	const double TimeLeft = TimeToTimeout - FPlatformTime::Seconds();
	const double TimeBetweenRetry = FMath::Min(0.001, TimeLeft / 32);
	const int NumberOfRetry = FMath::Max(1, FMath::FloorToInt(TimeLeft / TimeBetweenRetry));
	
	if (TimeLeft > 3600)
	{
		Lock<LockType>();
		return true;
	}
	
	for (int Retry = 0; Retry < NumberOfRetry; Retry++)
	{
		if (TimeToTimeout < FPlatformTime::Seconds())
		{
			return false;
		}
		if (TryLockImpl<LockType>())
		{
			return true;
		}
		if (TimeToTimeout < FPlatformTime::Seconds())
		{
			return false;
		}
		FPlatformProcess::Sleep(TimeBetweenRetry);
	}
	return false;
}