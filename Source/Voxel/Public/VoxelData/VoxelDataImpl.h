// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class FVoxelData;
struct FVoxelIntBox;

struct FModifiedValueDummy
{
	template<typename... TArgs>
	FModifiedValueDummy(TArgs...) {}
};

template<typename TModifiedValue = FModifiedValueDummy, typename TOtherModifiedValue = FModifiedValueDummy>
class TVoxelDataImpl
{
public:
	FVoxelData& Data;
	const bool bMultiThreadedEdits;
	const bool bRecordModifiedValues;
	TArray<TModifiedValue> ModifiedValues;
	TArray<TOtherModifiedValue> OtherModifiedValues;

	explicit TVoxelDataImpl(FVoxelData& Data, bool bMultiThreadedEdits, bool bRecordModifiedValues)
		: Data(Data)
		, bMultiThreadedEdits(bMultiThreadedEdits)
		, bRecordModifiedValues(bRecordModifiedValues)
	{
	}
	
	template<typename T, typename TLambda>
	void Set(const FVoxelIntBox& Bounds, TLambda Lambda);
	
	template<typename TA, typename TB, typename TLambda>
	void Set(const FVoxelIntBox& Bounds, TLambda Lambda);
};