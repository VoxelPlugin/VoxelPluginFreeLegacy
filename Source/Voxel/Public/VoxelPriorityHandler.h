// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"

// Somewhat thread safe array
class FInvokerPositionsArray
{
public:
	FInvokerPositionsArray() = default;
	explicit FInvokerPositionsArray(int32 NewMax)
		: Max(NewMax)
		, Data(reinterpret_cast<FIntVector*>(FMemory::Malloc(sizeof(FIntVector) * NewMax, alignof(FIntVector))))
	{
	}
	~FInvokerPositionsArray()
	{
		FMemory::Free(Data);
	}

	void Set(const TArray<FIntVector>& Array)
	{
		check(Array.Num() <= Max);
		for (int32 Index = 0; Index < Array.Num(); Index++)
		{
			Data[Index] = Array[Index];
		}
		// Make sure all the data is written before updating Num
		FPlatformMisc::MemoryBarrier();
		Num = Array.Num();
		// Force Num update
		FPlatformMisc::MemoryBarrier();
	}
	FORCEINLINE int32 GetMax() const
	{
		return Max;
	}
	FORCEINLINE int32 GetNum() const
	{
		return Num;
	}
	FORCEINLINE FIntVector Get(int32 Index) const
	{
		checkVoxelSlow(Index < Num);
		return Data[Index];
	}

private:
	int32 Num = 0;
	const int32 Max = 0;
	FIntVector* RESTRICT const Data = nullptr;
};

struct FVoxelPriorityHandler
{
	FVoxelIntBox Bounds;
	TVoxelSharedPtr<FInvokerPositionsArray> InvokersPositions;

	FVoxelPriorityHandler() = default;
	FVoxelPriorityHandler(const FVoxelIntBox& Bounds, const TVoxelSharedRef<FInvokerPositionsArray>& InvokersPositions)
		: Bounds(Bounds)
		, InvokersPositions(InvokersPositions)
	{
	}

	inline uint32 GetPriority() const
	{
		uint64 Distance = MAX_uint64;
		for (int32 Index = 0; Index < InvokersPositions->GetNum(); Index++)
		{
			const FIntVector Position = InvokersPositions->Get(Index);
			Distance = FMath::Min(Distance, Bounds.ComputeSquaredDistanceFromBoxToPoint(Position));
		}
		return MAX_uint32 - uint32(FMath::Sqrt(Distance));
	}
};