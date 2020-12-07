// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class VOXEL_API FVoxelLogTable
{
public:
	struct FEntry
	{
		FString Title;
		FString Value;
	};
	class FRow
	{
	public:
		template<typename T>
		FRow& Add(const FString& Title, const T& Value)
		{
			Columns.Add({ Title, LexToString(Value) });
			return *this;
		}

	private:
		TArray<FEntry> Columns;

		friend FVoxelLogTable;
	};

	FRow& AddRow()
	{
		return Rows.Emplace_GetRef();
	}

	TArray<FString> ToString() const;
	int32 NumRows() const { return Rows.Num(); }
	
private:
	TArray<FRow> Rows;
};