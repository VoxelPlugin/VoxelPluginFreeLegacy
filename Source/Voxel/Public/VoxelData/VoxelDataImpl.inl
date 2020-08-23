// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelData/VoxelDataImpl.h"
#include "VoxelData/VoxelData.inl"

template<typename TModifiedValue, typename TOtherModifiedValue>
template<typename T, typename TLambda>
void TVoxelDataImpl<TModifiedValue, TOtherModifiedValue>::Set(const FVoxelIntBox& Bounds, TLambda Lambda)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	if (bRecordModifiedValues)
	{
		FThreadSafeCounter Counter = ModifiedValues.AddUninitialized(Bounds.Count());

		const auto RecordingLambda = [&](int32 X, int32 Y, int32 Z, T& Value)
		{
			const T OldValue = Value;
			Lambda(X, Y, Z, Value);
			const T& NewValue = Value;

			if (OldValue != NewValue)
			{
				const int32 Index = Counter.Increment() - 1;
				if (ensureVoxelSlow(Index < ModifiedValues.Num()))
				{
					checkVoxelSlow(0 <= Index);
					ModifiedValues.GetData()[Index] = TModifiedValue{ FIntVector(X, Y, Z), OldValue, NewValue };
				}
			}
		};

		if (bMultiThreadedEdits)
		{
			Data.ParallelSet<T>(Bounds, RecordingLambda);
		}
		else
		{
			Data.Set<T>(Bounds, RecordingLambda);
		}

		ModifiedValues.SetNum(Counter.GetValue());
	}
	else
	{
		if (bMultiThreadedEdits)
		{
			Data.ParallelSet<T>(Bounds, Lambda);
		}
		else
		{
			Data.Set<T>(Bounds, Lambda);
		}
	}
}

template<typename TModifiedValue, typename TOtherModifiedValue>
template<typename TA, typename TB, typename TLambda>
void TVoxelDataImpl<TModifiedValue, TOtherModifiedValue>::Set(const FVoxelIntBox& Bounds, TLambda Lambda)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	if (bRecordModifiedValues)
	{
		FThreadSafeCounter CounterA = ModifiedValues.AddUninitialized(Bounds.Count());
		FThreadSafeCounter CounterB = OtherModifiedValues.AddUninitialized(Bounds.Count());

		const auto RecordingLambda = [&](int32 X, int32 Y, int32 Z, TA& ValueA, TB& ValueB)
		{
			const TA OldValueA = ValueA;
			const TB OldValueB = ValueB;
			Lambda(X, Y, Z, ValueA, ValueB);
			const TA& NewValueA = ValueA;
			const TB& NewValueB = ValueB;

			if (OldValueA != NewValueA)
			{
				const int32 Index = CounterA.Increment() - 1;
				if (ensureVoxelSlow(Index < ModifiedValues.Num()))
				{
					checkVoxelSlow(0 <= Index);
					ModifiedValues.GetData()[Index] = TModifiedValue{ FIntVector(X, Y, Z), OldValueA, NewValueA };
				}
			}
			if (OldValueB != NewValueB)
			{
				const int32 Index = CounterB.Increment() - 1;
				if (ensureVoxelSlow(Index < OtherModifiedValues.Num()))
				{
					checkVoxelSlow(0 <= Index);
					OtherModifiedValues.GetData()[Index] = TOtherModifiedValue{ FIntVector(X, Y, Z), OldValueB, NewValueB };
				}
			}
		};

		if (bMultiThreadedEdits)
		{
			Data.ParallelSet<TA, TB>(Bounds, RecordingLambda);
		}
		else
		{
			Data.Set<TA, TB>(Bounds, RecordingLambda);
		}

		ModifiedValues.SetNum(CounterA.GetValue());
		OtherModifiedValues.SetNum(CounterB.GetValue());
	}
	else
	{
		if (bMultiThreadedEdits)
		{
			Data.ParallelSet<TA, TB>(Bounds, Lambda);
		}
		else
		{
			Data.Set<TA, TB>(Bounds, Lambda);
		}
	}
}