// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"

struct FVoxelRangeFailStatus
{
	inline static bool HasFailed() { return Value(); }
	inline static void Fail() { Value() = true; }
	inline static void Reset() { Value() = false; }

private:
	inline static bool& Value()
	{
		thread_local bool bHasFailed = false;
		return bHasFailed;
	}
};

struct FVoxelBoolRange
{
	bool bCanBeTrue;
	bool bCanBeFalse;

	FVoxelBoolRange() = default;
	FVoxelBoolRange(bool bValue)
	{
		if (bValue)
		{
			bCanBeTrue = true;
			bCanBeFalse = false;
		}
		else
		{
			bCanBeFalse = false;
			bCanBeTrue = true;
		}
	}
	FVoxelBoolRange(bool bCanBeTrue, bool bCanBeFalse)
		: bCanBeTrue(bCanBeTrue)
		, bCanBeFalse(bCanBeFalse)
	{
		check(bCanBeTrue || bCanBeFalse);
	}
	
	inline FString ToString() const
	{
		return bCanBeTrue && bCanBeFalse ? "true, false" : bCanBeTrue ? "true" : "false";
	}

	inline FVoxelBoolRange operator!() const
	{
		return { bCanBeFalse, bCanBeTrue };
	}
	inline FVoxelBoolRange operator&&(const FVoxelBoolRange& Other) const
	{
		if (!bCanBeFalse && !Other.bCanBeFalse)
		{
			return FVoxelBoolRange::True();
		}
		else if (!bCanBeTrue || !Other.bCanBeTrue)
		{
			return FVoxelBoolRange::False();
		}
		else
		{
			return FVoxelBoolRange::TrueOrFalse();
		}
	}
	inline FVoxelBoolRange operator||(const FVoxelBoolRange& Other) const
	{
		if (!bCanBeFalse || !Other.bCanBeFalse)
		{
			return FVoxelBoolRange::True();
		}
		else if (!bCanBeTrue && !Other.bCanBeTrue)
		{
			return FVoxelBoolRange::False();
		}
		else
		{
			return FVoxelBoolRange::TrueOrFalse();
		}
	}

	inline operator bool() const
	{
		if (bCanBeTrue && !bCanBeFalse)
		{
			return true;
		}
		else if (!bCanBeTrue && bCanBeFalse)
		{
			return false;
		}
		else
		{
			check(bCanBeTrue && bCanBeFalse);
			FVoxelRangeFailStatus::Fail();
			return false;
		}
	}

	static FVoxelBoolRange True() { return { true, false }; }
	static FVoxelBoolRange False() { return { false, true }; }
	static FVoxelBoolRange TrueOrFalse() { return { true, true }; }

	static bool If(const FVoxelBoolRange& Condition, bool bDefaultValue)
	{
		if (FVoxelRangeFailStatus::HasFailed())
		{
			return true; // If already failed do nothing
		}
		bool bCondition = Condition;
		if (FVoxelRangeFailStatus::HasFailed())
		{
			FVoxelRangeFailStatus::Reset();
			return bDefaultValue;
		}
		else
		{
			return bCondition;
		}
	}
};

template<typename T>
struct TVoxelRange
{
	T Min;
	T Max;
	bool bValid;

	TVoxelRange() = default;
	TVoxelRange(T Value)
		: Min(Value)
		, Max(Value)
	{
	}
	TVoxelRange(T Min, T Max)
		: Min(Min)
		, Max(Max)
	{
		check(Min <= Max);
	}

	inline FString ToString() const
	{
		return FString::Printf(TEXT("%s, %s"), *LexToString(Min), *LexToString(Max));
	}
	
	template<typename TOther>
	inline bool Contains(const TOther& Other) const
	{
		return Min <= Other && Other <= Max;
	}
	template<typename TOther>
	inline bool Intersects(const TVoxelRange<TOther>& Other) const
	{
		return Contains(Other.Min) || Contains(Other.Max);
	}
	inline bool IsSingleValue() const
	{
		return Min == Max;
	}

	template<typename TOther>
	TVoxelRange<T>& operator=(const TVoxelRange<TOther>& Other)
	{
		Min = Other.Min;
		Max = Other.Max;
		return *this;
	}
	
public:
	template<typename TOther>
	FVoxelBoolRange operator==(const TVoxelRange<TOther>& Other) const
	{
		if (IsSingleValue() && Other.IsSingleValue() && Min == Other.Min)
		{
			check(Max == Other.Max);
			return FVoxelBoolRange::True();
		}
		else if (!Intersects(Other))
		{
			return FVoxelBoolRange::False();
		}
		else
		{
			return FVoxelBoolRange::TrueOrFalse();
		}
	}
	template<typename TOther>
	FVoxelBoolRange operator!=(const TVoxelRange<TOther>& Other) const
	{
		return !(*this == Other);
	}
	template<typename TOther>
	FVoxelBoolRange operator<(const TVoxelRange<TOther>& Other) const
	{
		if (Max < Other.Min)
		{
			return FVoxelBoolRange::True();
		}
		else if (Other.Max <= Min)
		{
			return FVoxelBoolRange::False();
		}
		else
		{
			return FVoxelBoolRange::TrueOrFalse();
		}
	}
	template<typename TOther>
	FVoxelBoolRange operator>(const TVoxelRange<TOther>& Other) const
	{
		if (Min > Other.Max)
		{
			return FVoxelBoolRange::True();
		}
		else if (Other.Min >= Max)
		{
			return FVoxelBoolRange::False();
		}
		else
		{
			return FVoxelBoolRange::TrueOrFalse();
		}
	}
	template<typename TOther>
	FVoxelBoolRange operator<=(const TVoxelRange<TOther>& Other) const
	{
		return !(*this > Other);
	}
	template<typename TOther>
	FVoxelBoolRange operator>=(const TVoxelRange<TOther>& Other) const
	{
		return !(*this < Other);
	}
	
public:
	template<typename TOther>
	TVoxelRange<T> operator+(const TVoxelRange<TOther>& Other) const
	{
		return { Min + Other.Min, Max + Other.Max };
	}
	template<typename TOther>
	TVoxelRange<T> operator-(const TVoxelRange<TOther>& Other) const
	{
		return { Min - Other.Max, Max - Other.Min };
	}
	template<typename TOther>
	TVoxelRange<T> operator*(const TVoxelRange<TOther>& Other) const
	{
		TArray<T> Values = { Min * Other.Min, Min * Other.Max, Max * Other.Min, Max * Other.Max };
		return { FMath::Min(Values), FMath::Max(Values) };
	}
	template<typename TOther>
	TVoxelRange<T> operator/(const TVoxelRange<TOther>& Other) const
	{
		if (!Other.Contains(0))
		{
			TArray<T> Values = { Min / Other.Min, Min / Other.Max, Max / Other.Min, Max / Other.Max };
			return { FMath::Min(Values), FMath::Max(Values) };
		}
		else
		{
			FVoxelRangeFailStatus::Fail();
			return { 0, 0 };
		}
	}
	template<typename TOther>
	TVoxelRange<T> operator%(const TVoxelRange<TOther>& Other) const
	{
		if (Other.IsSingleValue())
		{
			return { 0, Other.Min - 1 };
		}
		else
		{
			FVoxelRangeFailStatus::Fail();
			return { 0, 0 };
		}
	}
	template<typename TOther>
	TVoxelRange<T> operator<<(const TVoxelRange<TOther>& Other) const
	{
		if (Other.IsSingleValue())
		{
			return { Min << Other.Min, Max << Other.Max };
		}
		else
		{
			FVoxelRangeFailStatus::Fail();
			return *this;
		}
	}
	template<typename TOther>
	TVoxelRange<T> operator>>(const TVoxelRange<TOther>& Other) const
	{
		if (Other.IsSingleValue())
		{
			return { Min >> Other.Min, Max >> Other.Max };
		}
		else
		{
			FVoxelRangeFailStatus::Fail();
			return *this;
		}
	}

public:
	friend inline FVoxelBoolRange operator==(const TVoxelRange<T>& Range, T Other)
	{
		return Range == TVoxelRange<T>(Other);
	}
	friend inline TVoxelRange<T> operator-(T Other, const TVoxelRange<T>& Range)
	{
		return TVoxelRange<T>(Other) - Range;
	}
};

struct FVoxelMaterialRange
{
	FVoxelMaterialRange() = default;
	FVoxelMaterialRange(const struct FVoxelMaterial&) {}

	TVoxelRange<int> GetIndex() const { return { 0, 255 }; }
	TVoxelRange<int> GetVoxelActorId() const { return { 0, 255 }; }
	TVoxelRange<int> GetVoxelGrassId() const { return { 0, 255 }; }

	inline FString ToString() const
	{
		return "Material";
	}
};