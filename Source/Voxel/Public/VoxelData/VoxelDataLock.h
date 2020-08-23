// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelData.h"
#include "VoxelOctreeId.h"

class FVoxelDataLockInfo
{
public:
	~FVoxelDataLockInfo()
	{
		checkf(LockedOctrees.Num() == 0, TEXT("Data not unlocked by %s!"), *Name.ToString());
	}
	
	FVoxelDataLockInfo(const FVoxelDataLockInfo&) = delete;
	FVoxelDataLockInfo& operator=(const FVoxelDataLockInfo&) = delete;

private:
	FVoxelDataLockInfo() = default;
	
	FName Name;
	EVoxelLockType LockType = EVoxelLockType::Read;
	TArray<FVoxelOctreeId> LockedOctrees; // In depth first order
	
	friend class FVoxelData;
};

template<EVoxelLockType LockType>
class TVoxelScopeLock
{
public:
	using TData = typename TChooseClass<LockType == EVoxelLockType::Read, const FVoxelData, FVoxelData>::Result;
	
	TVoxelScopeLock(TData& InData, const FVoxelIntBox& Bounds, const FName& Name, bool bCondition = true)
		: Data(InData)
	{
		if (bCondition)
		{
			LockInfo = Data.Lock(LockType, Bounds, Name);
		}
	}
	~TVoxelScopeLock()
	{
		if (LockInfo.IsValid())
		{
			Unlock();
		}
	}

	void Unlock()
	{
		check(LockInfo.IsValid());
		Data.Unlock(MoveTemp(LockInfo));
	}

private:
	const FVoxelData& Data;
	TUniquePtr<FVoxelDataLockInfo> LockInfo;
};

class FVoxelReadScopeLock : public TVoxelScopeLock<EVoxelLockType::Read>
{
	using TVoxelScopeLock<EVoxelLockType::Read>::TVoxelScopeLock;
};
class FVoxelWriteScopeLock : public TVoxelScopeLock<EVoxelLockType::Write>
{
	using TVoxelScopeLock<EVoxelLockType::Write>::TVoxelScopeLock;
};

// Read lock that can be promoted to a write lock
class FVoxelPromotableReadScopeLock
{
public:
	FVoxelPromotableReadScopeLock(FVoxelData& Data, const FVoxelIntBox& Bounds, const FName& Name)
		: Data(Data)
		, Bounds(Bounds)
		, Name(Name)
	{
		LockInfo = Data.Lock(EVoxelLockType::Read, Bounds, Name);
	}
	~FVoxelPromotableReadScopeLock()
	{
		Data.Unlock(MoveTemp(LockInfo));
	}

	FORCEINLINE bool IsPromoted() const
	{
		return bPromoted;
	}
	void Promote()
	{
		check(!bPromoted);
		bPromoted = true;

		Data.Unlock(MoveTemp(LockInfo));
		LockInfo = Data.Lock(EVoxelLockType::Write, Bounds, Name);
	}

private:
	const FVoxelData& Data;
	const FVoxelIntBox Bounds;
	const FName Name;

	bool bPromoted = false;
	TUniquePtr<FVoxelDataLockInfo> LockInfo;
};