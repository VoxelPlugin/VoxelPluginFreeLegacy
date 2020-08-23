// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"

template<typename ItemType, EQueueMode Mode>
struct TVoxelQueueWithNum
{
	inline bool Dequeue(ItemType& OutItem)
	{
		const bool bSuccess = Queue.Dequeue(OutItem);
		if (bSuccess)
		{
			ensure(QueueNum.Decrement() >= 0);
		}
		return bSuccess;
	}
	inline void Empty()
	{
		Queue.Empty();
	}
	inline void Enqueue(const ItemType& Item)
	{
		QueueNum.Increment();
		Queue.Enqueue(Item);
	}
	inline void Enqueue(ItemType&& Item)
	{
		QueueNum.Increment();
		Queue.Enqueue(MoveTemp(Item));
	}
	inline bool IsEmpty() const
	{
		return Queue.IsEmpty();
	}
	inline 	bool Peek(ItemType& OutItem) const
	{
		return Queue.Peek(OutItem);
	}
	inline bool Pop()
	{
		const bool bSuccess = Queue.Pop();
		if (bSuccess)
		{
			ensure(QueueNum.Decrement() >= 0);
		}
		return bSuccess;
	}
	inline int32 Num() const
	{
		return QueueNum.GetValue();
	}

private:
	FThreadSafeCounter QueueNum;
	TQueue<ItemType, Mode> Queue;
};