// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

template<typename TValue = bool, typename TKey = uint64>
class TVoxelUniqueError
{
public:
	TVoxelUniqueError() = default;

	bool NeedToRaiseWarning(TKey Key, TValue Value)
	{
		auto& Set = Map.FindOrAdd(Key);
		if (!Set.Contains(Value))
		{
			Set.Add(Value);
			return true;
		}
		else
		{
			return false;
		}
	}
	bool operator()(TKey Key, TValue Value)
	{
		return NeedToRaiseWarning(Key, Value);
	}

private:
	TMap<TKey, TSet<TValue>> Map;
};