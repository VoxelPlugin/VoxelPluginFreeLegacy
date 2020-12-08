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
		FString SortValue;
		double Percent = 0;
	};
	class FRow
	{
	public:
		template<typename T, typename U>
		FRow& Add(const FString& Title, const T& Value, const U& SortValue, double Percent = 0)
		{
			Columns.Add({ Title, LexToString(Value), LexToString(SortValue), Percent});
			return *this;
		}
		template<typename T>
		FRow& Add(const FString& Title, const T& Value)
		{
			return Add(Title, Value, Value);
		}
		FRow& AddNumber(const FString& Title, int64 Value, TOptional<int64> Total = {})
		{
			if (Total.IsSet())
			{
				const double Percent = GetPercent(Value, Total.GetValue());
				return Add(Title, FString::Printf(TEXT("%lld (%0.3f%%)"), Value, Percent), Value, Percent);
			}
			else
			{
				return Add(Title, FString::Printf(TEXT("%lld"), Value), Value);
			}
		}
		FRow& AddSeconds(const FString& Title, double Value, TOptional<double> Total = {})
		{
			if (Total.IsSet())
			{
				const double Percent = GetPercent(Value, Total.GetValue());
				return Add(Title, FString::Printf(TEXT("%0.3fs (%0.3f%%)"), Value, Percent), Value, Percent);
			}
			else
			{
				return Add(Title, FString::Printf(TEXT("%0.3fs"), Value), Value);
			}
		}
		FRow& AddNanoseconds(const FString& Title, double Value)
		{
			return Add(Title, FString::Printf(TEXT("%0.3fns"), Value), Value);
		}

	private:
		TArray<FEntry> Columns;

		static double GetPercent(double A, double B)
		{
			return 100 * A / double(B);
		}

		friend FVoxelLogTable;
	};

	FRow& AddRow()
	{
		return Rows.Emplace_GetRef();
	}

	TArray<FString> ToString(bool bHtml = false) const;
	int32 NumRows() const { return Rows.Num(); }
	
private:
	TArray<FRow> Rows;
};